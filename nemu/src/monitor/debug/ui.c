#include "monitor/monitor.h"
#include "monitor/expr.h"
#include "monitor/watchpoint.h"
#include "nemu.h"

#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>

void cpu_exec(uint32_t);

/* We use the `readline' library to provide more flexibility to read from stdin. */
char* rl_gets() {
	static char *line_read = NULL;

	if (line_read) {
		free(line_read);
		line_read = NULL;
	}

	line_read = readline("(nemu) ");

	if (line_read && *line_read) {
		add_history(line_read);
	}

	return line_read;
}

static int cmd_c(char *args) {
	cpu_exec(-1);
	return 0;
}

static int cmd_q(char *args) {
	return -1;
}

static int cmd_help(char *args);
static int cmd_si(char *args);
static int cmd_info(char *args);
static int cmd_x(char *args);

static struct {
	char *name;
	char *description;
	int (*handler) (char *);
} cmd_table [] = {
	{ "help", "Display informations about all supported commands", cmd_help },
	{ "c", "Continue the execution of the program", cmd_c },
	{ "q", "Exit NEMU", cmd_q },
        { "si", "The program executes N instructions in a single step and then pause", cmd_si },
        { "info", "Info register state", cmd_info },
        { "x", "Scanning memory", cmd_x },
	/* TODO: Add more commands */

};

#define NR_CMD (sizeof(cmd_table) / sizeof(cmd_table[0]))
static int cmd_x(char *args){
        char *arg1 = strtok(NULL," ");
        char *arg2 = strtok(NULL," ");
        int length;
        lnaddr_t address;
    
        sscanf(arg1,"%d",&length);
        sscanf(arg2,"%x",&address);

        int i;
        printf("0x%x:",address);
        for(i=0;i<length;i++){
            printf("0x%x ",lnaddr_read(address,4));
            address+=4;
        }
        return 0;        
}

static int cmd_info(char *args){
        char *arg = strtok(NULL," ");
        
        if(strcmp(arg,"r")==0){
            printf("eax 0x%x \n",cpu.gpr[0]._32);
            printf("ecx 0x%x \n",cpu.gpr[1]._32);
            printf("edx 0x%x \n",cpu.gpr[2]._32);
            printf("ebx 0x%x \n",cpu.gpr[3]._32);
            printf("esp 0x%x \n",cpu.gpr[4]._32);
            printf("ebp 0x%x \n",cpu.gpr[5]._32);
            printf("esi 0x%x \n",cpu.gpr[6]._32);
            printf("edi 0x%x \n",cpu.gpr[7]._32);
        }

        return 0;
}

static int cmd_si(char *args){
        char *arg = strtok(NULL," ");
        int i;
        if(arg == NULL){
            cpu_exec(1);
            return 0;
        }
        
        sscanf(arg,"%d",&i);
        
        if(i<-1){
            printf("error\n");
            return 0;
        }
        
        if(i==-1){
            cpu_exec(-1);
        }
        int j;
        for(j=0;j<i;j++){
            cpu_exec(1);
        }
        return 0;
        
}

static int cmd_help(char *args) {
	/* extract the first argument */
	char *arg = strtok(NULL, " ");
	int i;

	if(arg == NULL) {
		/* no argument given */
		for(i = 0; i < NR_CMD; i ++) {
			printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
		}
	}
	else {
		for(i = 0; i < NR_CMD; i ++) {
			if(strcmp(arg, cmd_table[i].name) == 0) {
				printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
				return 0;
			}
		}
		printf("Unknown command '%s'\n", arg);
	}
	return 0;
}

void ui_mainloop() {
	while(1) {
		char *str = rl_gets();
		char *str_end = str + strlen(str);

		/* extract the first token as the command */
		char *cmd = strtok(str, " ");
		if(cmd == NULL) { continue; }

		/* treat the remaining string as the arguments,
		 * which may need further parsing
		 */
		char *args = cmd + strlen(cmd) + 1;
		if(args >= str_end) {
			args = NULL;
		}

#ifdef HAS_DEVICE
		extern void sdl_clear_event_queue(void);
		sdl_clear_event_queue();
#endif

		int i;
		for(i = 0; i < NR_CMD; i ++) {
			if(strcmp(cmd, cmd_table[i].name) == 0) {
				if(cmd_table[i].handler(args) < 0) { return; }
				break;
			}
		}

		if(i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
	}
}
