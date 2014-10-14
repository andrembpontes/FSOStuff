#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <sys/wait.h>

#define	MAX_CMD_LINE					80		//Max cmd line size
#define MAX_ARGS						10		//Max number of each cmd args
#define PROMPT							"> "	//Prompt string
#define INTERNAL_CMD_S					2		//Number of internal cmds
#define ENABLE_INTERNAL_CMD 			0		//Enable internal cmds? 0 = Off

char* INTERNAL_CMD[] = {"exit", "cd"};			//Internal cmds
char* NEXT_CMD_BUF;								//Buff used to read a cmd expression (with &&)

int get_internal_cmd_i(char* cmd){
	int i;
	for(i = 0; i < INTERNAL_CMD_S; i++){
		if(! strcmp(INTERNAL_CMD[i], cmd)){
			return i;
		}
	}

	return -1;
}

int is_internal_cmd(char* cmd){
	if(ENABLE_INTERNAL_CMD){
		int i;
		for(i = 0; i < INTERNAL_CMD_S; i++){
			if(! strcmp(INTERNAL_CMD[i], cmd)){
				return 1;
			}
		}
	}

	return 0;
}

void create_argv(char* cmd, char** argv){
	int i = 0;
	argv[i++] = strtok(cmd, " \n\t");
	while(i < MAX_ARGS){
		argv[i++] = strtok(NULL, " \n\t");
	}
}

int print_args(char* argv[]){
	int i = 0;
	while(i <= MAX_ARGS && argv[i] != NULL){
		printf("arg[%d] = %s\n", i, argv[i]);
		i++;
	}

	return i;
}

void execute_external_cmd(char* argv[]){
	int child_pid = fork();

	if(child_pid < 0){
		printf("FATAL ERROR! : fork failed!!");
		exit(0);
	}

	if(child_pid){
		//father execution
		wait(NULL);
	}
	else{
		//child execution
		execvp(argv[0], argv);
		printf("Command not found! :(\n");
	}
}

void exec_exit(char* argv[]){
	exit(0);
}

void exec_cd(char* argv[]){
	printf("internal command cd executed\n");
	print_args(argv);
}

void execute_internal_cmd(char* argv[]){
	switch(get_internal_cmd_i(argv[0])){
		case 0: exec_exit(argv); break;
		case 1: exec_cd(argv); break;
		default: printf("internal command not yet implemented! :(\n");
	}
}

void execute_command(char* cmd){

	char* argv[MAX_ARGS + 1];

	create_argv(cmd, argv);
	//print_args(argv);

	if(argv[0]){
		if(is_internal_cmd(argv[0])){
			execute_internal_cmd(argv);
		}
		else{
			execute_external_cmd(argv);
		}
	}

}

char* next_cmd(char* cmd_line){
	if(cmd_line == NULL){
		char* toReturn = NEXT_CMD_BUF;
		while(*NEXT_CMD_BUF != '\0'){
			if(*NEXT_CMD_BUF == '&' && *(NEXT_CMD_BUF + 1) == '&') {
				*NEXT_CMD_BUF = '\0';
				NEXT_CMD_BUF += 2; // (NEXT_CMD_BUF + 2);
				break;
			}
			NEXT_CMD_BUF ++;
		}
		return toReturn;

	}
	else{
		NEXT_CMD_BUF = cmd_line;
		return next_cmd(NULL);
	}
}

void execute_command_line(char* cmd_line){
	char* cmd = next_cmd(cmd_line);

	while(*cmd){
		execute_command(cmd);
		cmd = next_cmd(NULL);
	}
}

void read_command_line(char* cmd_line){
	fgets(cmd_line, MAX_CMD_LINE, stdin);
}

void print_prompt(){
	printf("%s", PROMPT);
	fflush(stdin);
}

int main(int argc, char* argv[]){

	char* cmd_line = malloc(MAX_CMD_LINE);

	while(1){
		print_prompt();
		read_command_line(cmd_line);
		execute_command_line(cmd_line);
	}

	return 0;
}
