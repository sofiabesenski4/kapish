/* kapish.c
 * Thomas Besenski
 * V00879481
 * Lots of the ideas in here are inspired by https://brennan.io/2015/01/16/write-a-shell-in-c/
 * 
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>
#define BUFFERSIZE 1024
int setenv(const char *name, const char *value, int overwrite);
int unsetenv(const char *name);



void kapish_sigint_handler(int sig_num)
{
	signal(SIGINT,kapish_sigint_handler);
	
	printf("enter 'exit' or CTRL+D to kill kapish. To continue, press enter and/or type your commands when the prompt resets\n");
	fflush(stdin);
}

static void child_sigint_handler(int sig_num) 
{ 
    /* Reset handler to catch SIGINT next time. 
       Refer http://en.cppreference.com/w/c/program/signal */
    signal(SIGINT, kapish_sigint_handler); 
    printf("\n terminating child process \n"); 
    //fflush(stdout);
    sleep(2); 
    exit(0);
} 

char* take_input(void)
{
	//while ((getchar()) != '\n'); 
	//printf("DEBUG:in take_input\n");
	//setting the buf_size such that we can make it larger if the input is too large
	//Note: the size_t represents a generic size type, which will never be a negative number 
	size_t buf_size = BUFFERSIZE*sizeof(char);
	//define the buffer we will copy the user input into
	char* buffer = malloc(buf_size);
	//reset the characters
	memset(buffer,'\0',buf_size);
	//if the buffer is a null pointer, then a mem allocation failed
	if (!buffer)
	{fprintf(stderr,"error while allocation in take_input\n");exit(EXIT_FAILURE);}
	//position points to the position of the next empty character in the buffer
	int position=0;
	//character holds the next literal value of the user's input, which is to be copied to the buffer
	char character;
	//awaiting user input
	printf ("? ");
	//enter the while look to interpret the characters one-by-one
	while(1)
	{
		if (feof(stdin))
		{
			char temp[] = "exit";
			memset(buffer,'\0',buf_size);
			memcpy(buffer,temp,buf_size);
			//printf("%s",buffer);
			return buffer;
		}
		
		//get the next user char
		character = getchar();
		//if it is a newline then the user input has ended 
		if (character=='\n')
		{
		//make sure the buffer's string is terminated by a NULL character
			buffer[position]='\n';
			return buffer;
		}
		else
		{
		//if the character is not a null one, then add it to the buffer and carry on interpretting
			buffer[position]=character;
		}
		position++;
		//if the position has passed the allotted character buffer, then reallocate the memory to a new spot which is 
		//larger by BUFFERSIZE*sizeof(char) where BUFFERSIZE is a constant 
		if (position>=buf_size)
		{
			buf_size += BUFFERSIZE*sizeof(char);
			buffer = realloc(buffer, buf_size);	
		}	
	}	
}

char** tokenize_input(char* line)
{
	//defining a bufersize to reference in this function.
	//necessary to not just use the constant so that we can realloc a larger memspace in case of bufferoverflow
	size_t tokens_buffer_size = BUFFERSIZE*sizeof(char*);
	//counter of pointers
	int i = 0;
	//delimiters which we are tokenizing the string by
	const char delims[]=" \n";
	//allocate enough memory for 
	char** tokens = malloc(tokens_buffer_size); 
	memset(tokens,'\0',tokens_buffer_size);
	char* token;
	if (!tokens){fprintf(stderr,"mem alloc failure in tokenize_input\n");exit(EXIT_FAILURE);}
	if (*line=='\0')
	{
		return NULL;
	}
	token = strtok(line,delims);
	while (token!=NULL)
	{
		//since strtok doesn't modify the position of the tokenized strings, it only adds a null character
		//afterwards, we are able to simply assign a pointer to that address in heap memory, allocated in take_input()
		tokens[i] = token;	
		
		//printf("in the 'line' buffer: %s\n",line);
		
		//incrementing the token/string pointer counter
		i++;
		
		//if the number of tokens has surpassed the number of available string pointers in tokens, then realloc to a larger 
		//set of token/string pointers
		if (i>=tokens_buffer_size)
		{
			tokens_buffer_size += tokens_buffer_size;
			tokens = realloc(tokens, tokens_buffer_size);
			if (!tokens){fprintf(stderr,"failed to realloc in tokenize_input");exit(EXIT_FAILURE);}	
		}
		//get the next token
		token=strtok(NULL,delims);
	}
	//make sure the last pointer in the list is a null pointer
	tokens[i]=NULL;
	//return the pointer to the tokenized strings 
	return tokens;
} 

int execute_shell_command(char **args)
{
	
	//printf("DEBUG: in execute_shell_command\n");
	pid_t pid, child_pid;
	int status_code;
	pid = fork();
	if (pid==0)
	{//will trigger if this process is the child process
		signal(SIGINT, child_sigint_handler);		
		
		//execvp will only return if there is an error in the execution of the process
		printf("currently in child process %d\n",getpid());
		//trying to set the signal handler
		if (execvp(args[0],args)==-1)
		{
			fprintf(stderr,"error in the execution of process %s\n",args[0]);exit(EXIT_FAILURE);}
		exit(EXIT_FAILURE);
		
	}
	else if (pid<0)
	{//encountered an error and a child process was never spawned
		fprintf(stderr,"child process was never spawned from arg %s\n",args[0]);
			
	}
	else
	{//this will only trigger if THIS process is the parent process, and the child was successfully spawned
	//so essentially, wait now for the child process to finish
		//int* status;
		child_pid = pid;
		waitpid(child_pid,&status_code,0);
		//if (status_code!=0)
		//{printf("child process %d exited successfully\n",(int)child_pid);}	
		return 0;
	}
	return 0;
}

/*
 * handle arguments takes a list of string commands,
 * and either processes the built in commands accordingly,
 * or it passes the arguments in to the execute_shell_command function
 * so another process can be spawned and handled. 
 * */
int handle_arguments(char **args)
{
	
	if (!args[0])
	{
		return 0;}
	//printf("DEBUG: in handle_arguments\n");
	//This is the list of built in commands that 
	//printf("args[0]: %s\n",args[0]);
	
	
	//cd
	if(!strcmp(args[0],"cd"))
	{
		if (
		
		args[1]==NULL)
		{
			//printf("HOME= %s\n",getenv("HOME"));
			if (chdir(getenv("HOME"))!=0)
			{fprintf(stderr,"error while cding to home\n");return 1;}
		}
		else
		{
			if (chdir(args[1])!=0)
			{
				fprintf(stderr,"error in cd\n");
				return 1;
			}
			return 0;
		}
	}
	//setenv
	else if (!strcmp(args[0],"setenv"))
	{
		
		if (args[1]==NULL)
		{
			fprintf(stderr,"no environment variable to add specified in setenv(variable, [value])\n");
			return 1;
		}
		//if there is a 'value' specified
		if (args[2]!=NULL)
		{	
			if (setenv(args[1],args[2],1)!=0)
			{fprintf(stderr,"error while setting env variable\n");return 1;}
			printf("successfully added env variable %s, with value %s\n",args[1],args[2]);
			return 0;
			
		}
		else
		{
			if (setenv(args[1],"",1)!=0)
			{fprintf(stderr,"failure while setting env variable %s to NULL\n",args[1]);return 1;}
			return 0;
		}
	}
	//unsetenv
	
	else if (!strcmp(args[0],"unsetenv"))
	{
		if (args[1]==NULL)
		{
			fprintf(stderr,"no environment variable to unset specified in unsetenv(variable)\n");
			return 1;	
		}
		else
		{
			if (unsetenv(args[1])!=0)
			{
				fprintf(stderr,"unable to unset the env variable %s\n",args[1]);return 1;
			}
			printf("successfully removed env variable %s\n",args[1]);
			return 0;
		}
	
	}
	
	
	//exit
	else if (!strcmp(args[0],"exit"))
	{//this is the only function that returns 2; the loop function will recognize this and break the loop
		return 2;
	}
	
	//This branch will attempt to execute the commands 
	else{
		//printf("about to execute_shell_commands\n");
		execute_shell_command(args);
		return 0;
	}
	
}



void shell_loop(void)
{
	char exit[]="exit";
	while (1) {
		
		char* input_line;
		int status;
	
		input_line = take_input();
		//printf("input: %s",input_line);
		
		if (!strcmp(input_line,exit))
		{
			//sleep(1);
			//printf("here");
			free(input_line);
			break;
		}


		char** args = tokenize_input(input_line);
		status = handle_arguments(args);
		free(args);
		free(input_line);
		
		//if handle_arguments encountered an exit command, then it will return 2
		if (status==2)
		{break;}
		/*
		char*  command_path = malloc(sizeof(char)*BUFFERSIZE);
		strncpy(command_path,"/bin/",BUFFERSIZE);
		strncat(command_path,cmd, BUFFERSIZE);
		
		int pid = fork();
		if (pid == 0) {
			pid_t process_id = getpid();
			printf("command_path: %s\n",command_path);
			printf("in child with pid= %d\n",process_id);
			printf("args: %s, %s\n", args[0],args[1]);
			printf("Executing command:\n");
			int stat  = execvp(command_path,args);
			printf("status:%d\n", stat);
			exit(0);
		} 
		else {
		wait(&pid);
		}
	
		*/
	}
	
	
	
	return;
}


//this function is for processing the rc file
void process_rc()
{

	char* filepath;
	filepath = getenv("HOME");
	char *rc_path = malloc(BUFFERSIZE*sizeof(char));
	strncpy(rc_path,filepath,BUFFERSIZE*sizeof(char));
	strncat(rc_path,"/.kapishrc",12); 
//rc_path now holds the path to the rc file which should be in the home directory

	FILE *fp;
	if ((fp = fopen(rc_path,"r"))==NULL)
	{	printf("error opening rc file!!\n");
		return;
	}
	char* temp = malloc(sizeof(char)*BUFFERSIZE);
	//now executing the commands in the .kapishrc file
	
	
	//char* line;
	char** args;
	
	while(fgets(temp,BUFFERSIZE,fp))
	{	//printf("%s",temp);
		printf("? %s",temp);
		args = tokenize_input(temp);
		handle_arguments(args);
		memset(temp,'\0',BUFFERSIZE*sizeof(char));
		free(args);
	}
	filepath=NULL;
	free(temp);
	free(rc_path);
	fclose(fp);	
}
int main(int argc, char **argv)
{
	signal(SIGINT,kapish_sigint_handler);
	process_rc();
	shell_loop();
	return EXIT_SUCCESS;	
}
