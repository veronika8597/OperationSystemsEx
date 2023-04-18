#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <wait.h>
#include <sys/stat.h>
#include <stdbool.h>    /*bool type */



#define MYSH_INPUT_BUFSIZE 1024 /* buffer size for reading user input */
#define MYSH_TOK_BUFSIZE 64 /* buffer size for splitting the arguments */
#define MYSH_TOK_DELIM " \t\r\n" /* delimiters for parsing the arguments */

//int bufsize = MYSH_INPUT_BUFSIZE;

bool file_exist (char *filename) {
    struct stat file_info_buf;
    return (stat(filename, &file_info_buf) == 0);
 }


int mysh_exit()
{
}

/* Parse input to get the arguments */
char **mysh_split_line(char *line){
    int position = 0;
    char **tokens = malloc(MYSH_INPUT_BUFSIZE * sizeof(char*));
    char *token;

    if(!tokens){
        fprintf(stderr, "mysh: allocation error\n");
        exit(EXIT_FAILURE);
    }

    token = strtok(line, MYSH_TOK_DELIM);
    while(token != NULL){
        tokens[position] = token;
        position++;

        if(position >= MYSH_INPUT_BUFSIZE){
                fprintf(stderr, "mysh: Input exceeds buuffer limit %d\n",MYSH_INPUT_BUFSIZE);
                exit(EXIT_FAILURE);
        }

        token = strtok(NULL, MYSH_TOK_DELIM);
    }
    tokens[position] = NULL;
    return tokens;
}


/* Execute the parsed arguments */
int mysh_execute(char *line){
    int ret=0;
    int status;
    pid_t pid;
    char *command = malloc(sizeof(char) * MYSH_INPUT_BUFSIZE);
    //printf("%s\n", line);

    char *dir_list = getenv("PATH");
    char **args = mysh_split_line(line);
    char *token;

    if(args[0] == NULL){ /* empty command was entered */
        return 1;
    }
    else if (strcmp(args[0], "leave") == 0) 
    {
	printf ("Got leave command. Bye Bye\n");
	return 0;
    }

    pid = fork();
    if (pid < 0)
    {
	fprintf(stderr, "Fork failed\n");
	perror ("mysh fork error:");
	exit(EXIT_FAILURE);
    }
    else if (pid == 0)   /* child process */
    {
	/* Find full path for the command executable */
	for (token = strtok(dir_list, ":"); token != NULL; token = strtok(NULL, ":"))
	{
	    sprintf(command, "%s/%s", token, args[0]);
	    if (file_exist(command)) 
	    {
		ret = execv(command, args);
		if (ret == -1)
		    perror("mysh execv error:");
		exit (ret);
	    }
	}
	fprintf(stderr, "command not recognized\n");
	exit(EXIT_FAILURE);
    }
    else 
    {
	wait(&status);
	fprintf (stdout, "Child process completed with status %d\n", status);
    }
    return 1;
}

/* Read input from stdin */
char *mysh_read_line(void)
{
    int position = 0;
    char *buffer = malloc(sizeof(char) * MYSH_INPUT_BUFSIZE);
    int c;

    if(!buffer){
        fprintf(stderr, "mysh: allocation error\n");
        exit(EXIT_FAILURE);
    }

    while(1){
        /* Read a character */
        c = getchar();

        if(c == EOF || c == '\n'){
            buffer[position] = '\0';
            return buffer;
        }else{
            buffer[position] = c;
        }
        position++;

    }
}

/* Loop for getting input and executing it */
void mysh_loop(void)
{
    char *line;
    int status;

    do {
        printf("mysh> ");
        line = mysh_read_line();
        status = mysh_execute(line);

        free(line);
    } while(status);
}

int main(void)
{
    mysh_loop();

    return EXIT_SUCCESS;
}
