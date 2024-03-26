/**********************************************************************
 * Copyright (c) 2021-2022
 *  Sang-Hoon Kim <sanghoonkim@ajou.ac.kr>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTIABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 **********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <unistd.h>
#include <errno.h>

#include <string.h>

#include "types.h"
#include "list_head.h"
#include "parser.h"

#include <sys/wait.h>
#include <signal.h>
/***********************************************************************
 * struct list_head history
 *
 * DESCRIPTION
 *   Use this list_head to store unlimited command history.
 */
extern struct list_head history;

struct entry{
	struct list_head list;
	char *command;
};

pid_t pid;
char* command;

void timeout(int a){
	if(a==SIGALRM){
		fprintf(stderr,"%s is timed out\n",command);
		kill(pid,SIGKILL);
	}
}

int time = 0;
int status;

/***********************************************************************
 * run_command()
 *
 * DESCRIPTION
 *   Implement the specified shell features here using the parsed
 *   command tokens.
 *
 * RETURN VALUE
 *   Return 1 on successful command execution
 *   Return 0 when user inputs "exit"
 *   Return <0 on error
 */
int run_command(int nr_tokens, char * const tokens[])
{
	command = tokens[0];
	struct sigaction sig;
	sig.sa_handler = timeout;
	sigemptyset(&sig.sa_mask);
	sig.sa_flags=0;
	sigaction(SIGALRM,&sig,0);

	if (strcmp(tokens[0], "exit") == 0) return 0;

	if(strcmp(tokens[0],"cd") == 0){

		if(nr_tokens == 1){
			chdir(getenv("HOME"));
		}else if(strcmp(tokens[1],"~") == 0){
			chdir(getenv("HOME"));
		}else{
			chdir(tokens[1]);
		}
	}else if(strcmp(tokens[0],"history") == 0){
		struct entry* p = NULL;
		int index = 0;

		list_for_each_entry_reverse(p,&history,list){
			fprintf(stderr,"%2d: %s",index,p->command);
			index += 1;
		}

	}else if(strcmp(tokens[0],"!") == 0){
		struct entry* p = NULL;
		int index = 0;
		char find_command[MAX_COMMAND_LEN];
		int token = atoi(tokens[1]);
		list_for_each_entry_reverse(p,&history,list){
			if(index == token)break;
			index++;
		}
		strcpy(find_command,p->command);

		char * new_tokens[MAX_NR_TOKENS]={NULL};
		int num = 0;
		if(parse_command(find_command,&num,new_tokens)==0)return -1;
		run_command(num,new_tokens);

	}else if(strcmp(tokens[0],"timeout")==0){

		if(nr_tokens == 1){
			fprintf(stderr,"Current timeout is 0 second\n");
		}else if(strcmp(tokens[1],"0")==0){
			fprintf(stderr,"Timeout is disable\n");
		}else{
			time = atoi(tokens[1]);
			fprintf(stderr,"Timeout is set to %d seconds\n",time);
		}
	}else{
		pid = fork();

		switch(pid){
			case -1:
				fprintf(stderr,"fork error\n");
				break;
			case 0:
				if(execvp(tokens[0],tokens)<0){
					fprintf(stderr,"Unable to execute %s\n",tokens[0]);
				exit(1);
				}
				break;
			default:
				alarm(time);
				waitpid(pid,&status,0);
				break;	
		}
		alarm(0);
	}
	return -EINVAL;
}


/***********************************************************************
 * append_history()
 *
 * DESCRIPTION
 *   Append @command into the history. The appended command can be later
 *   recalled with "!" built-in command
 */
void append_history(char * const command)
{
	struct entry* new = (struct entry*)malloc(sizeof(struct entry));
	new->command = malloc(sizeof(char)*MAX_COMMAND_LEN);
	strcpy(new->command,command);
	list_add(&new->list,&history);
}


/***********************************************************************
 * initialize()
 *
 * DESCRIPTION
 *   Call-back function for your own initialization code. It is OK to
 *   leave blank if you don't need any initialization.
 *
 * RETURN VALUE
 *   Return 0 on successful initialization.
 *   Return other value on error, which leads the program to exit.
 */
int initialize(int argc, char * const argv[])
{
	return 0;
}


/***********************************************************************
 * finalize()
 *
 * DESCRIPTION
 *   Callback function for finalizing your code. Like @initialize(),
 *   you may leave this function blank.
 */
void finalize(int argc, char * const argv[])
{

}


/***********************************************************************
 * process_command(command)
 *
 * DESCRIPTION
 *   Process @command as instructed.
 */
int process_command(char * command)
{
	char *tokens[MAX_NR_TOKENS] = { NULL };
	int nr_tokens = 0;

	if (parse_command(command, &nr_tokens, tokens) == 0)
		return 1;

	return run_command(nr_tokens, tokens);
}
