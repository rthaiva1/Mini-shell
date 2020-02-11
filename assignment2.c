#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include<fcntl.h>
#include <sys/wait.h>
#include<signal.h>

//limits
#define MAX_TOKENS 100
#define MAX_STRING_LEN 100

size_t MAX_LINE_LEN = 10000;

// builtin commands
#define EXIT_STR "exit"
#define EXIT_CMD 0
#define UNKNOWN_CMD 99

FILE *fp; // file struct for stdin
char **tokens;
char *line;
char *delimiter_token;
int flag = 0;
int noarg = 0;
char *temp[50];
char *temp_1[50];
int cmd_count = 0;
int input_flag = 0;
int output_flag = 0;
char inputfile[20];
char outputfile[20];
int in;
int out;
pid_t fg;
int kill_chain = 0;
struct Process_id
{
	pid_t pid[100];
  int stat[100];
} p;


void initialize()
{

	// allocate space for the whole line
	assert( (line = malloc(sizeof(char) * MAX_STRING_LEN)) != NULL);

	// allocate space for the individual command
	assert( (delimiter_token = malloc(sizeof(char) * MAX_STRING_LEN)) != NULL);

	// allocate space for individaual tokens
	assert( (tokens = malloc(sizeof(char*)*MAX_TOKENS)) != NULL);

	// open stdin as a file pointer
	assert( (fp = fdopen(STDIN_FILENO, "r")) != NULL);

}

void tokenize (char * string)
{
	int token_count = 0;
	int size = MAX_TOKENS;
	char *this_token;

	while ( (this_token = strsep( &string, " \t\v\f\n\r")) != NULL)
	{
		if (*this_token == '\0') continue;
		if(strcmp(this_token,"<") == 0)
		{
			input_flag = 1;
		}
		else if(input_flag == 1)
		{
			strcpy(inputfile,this_token);
		}
		else if(strcmp(this_token,">") == 0)
		{
			output_flag = 1;
		}
		else if(output_flag == 1)
		{
			strcpy(outputfile,this_token);
		}
		else
		{
			tokens[token_count] = this_token;
			token_count++;
		}
		if(token_count >= size){
			size*=2;

			assert ( (tokens = realloc(tokens, sizeof(char*) * size)) != NULL);
		}
	}
	if(token_count == 0)
	{
		noarg = 1;
		return;
	}
	int i;
	for(i =0; i< token_count ; i++)
	{
		temp[i]=tokens[i];
	}
	if(strcmp(temp[token_count-1],"&") == 0)
	{
		flag=1;
		temp[token_count-1] = NULL;
	}
	else
	{
				temp[token_count] = NULL;
	}
}

void read_command()
{
	assert( getline(&line, &MAX_LINE_LEN, fp) > -1);
}

void handler(int sig)
{
	printf("\nThe foreground process is terminated with its chain of filters\n");
	kill(fg,SIGKILL);
	kill_chain = 1;
}

int i;
void track(int sig)
{
	pid_t h;
	int s;
	while((h=waitpid(-1, &s, WNOHANG)) != -1)
	{
		break;
	}
	for(i=0;i<100;i++)
	{
		if(p.pid[i] == h)
		{
			p.stat[i] = 1;
			break;
		}
	}
}

void listjobs_method()
{
	i=0;
	while(p.pid[i]>0 )
	{
		if(p.stat[i]!= -1)
		{
		printf("List of background processes:\n");
		printf("Command %d with PID %d Status: ",i+1,p.pid[i]);
		if (p.stat[i] == 1)
		{
			printf("FINISHED\n");
		}
		else if (p.stat[i] == 0)
		{
			printf("RUNNING\n");
		}
		}
		i++;
	}
}

int main()
{
	initialize();
	signal(SIGINT,handler);
	while(1) {
		printf("sh550> ");
		read_command();
		delimiter_token = strtok(line, "|");
		temp_1[0] = delimiter_token;
		 while (delimiter_token != NULL)
		 {
				 cmd_count++;
				 delimiter_token = strtok(NULL, "|");
				 temp_1[cmd_count] = delimiter_token;
		 }
		 int x;
		 int rfd;
		 int fds[2];
		 pid_t id;
		 int status;
		 x= 0;
		 do
		 {
			 if(kill_chain == 1)
			 {
				 kill_chain = 0;
				 break;
			 }
			 if(pipe(fds) == -1)
			 {
				 return 0;
			 }
			 tokenize(temp_1[x]);
		 	if(noarg == 1)
		 	{
		 		noarg = 0;
		 		break;
		 	}
		 	if (strcmp( temp[0], EXIT_STR ) == 0)
		 	{
		 		  return 0;
		 	}
			if (strcmp( temp[0],"kill") == 0)
			{
					kill(atoi(temp[1]), SIGKILL);
			}
			else
		 	if (strcmp(temp[0],"fg") == 0)
		 	{
				waitpid(atoi(temp[1]),&status,0);
				for(i=0;i<100;i++)
				{
					if(p.pid[i] == atoi(temp[1]))
					{
						p.stat[i] = -1;
					}
				}
		 	}
		 	else if (strcmp( temp[0],"listjobs") == 0)
		 	{
		 		listjobs_method();
		 	}
			else
			{
		 	id = fork();
			fg = id;
		 	if (id > 0)
		 	{
		 	for(i = 0;i<100;i++)
		 	{
		 		if(p.pid[i] <= 0)
		 		{
		 			p.pid[i]=id;
		 		 break;
		 		}
		 	}
		 	}
		 	if (id < 0)
		 	{
		 		exit(0);
		 	}
		 	else if (id == 0)
		 	{
		 			if(input_flag == 1)
		 			{
		 				in = open(inputfile,O_RDONLY);
		 				dup2(in,0);
						close(fds[0]);
						input_flag=0;
		 			}
					else if(x > 0)
					{
					 dup2(rfd,0);
					 close(fds[0]);
					}
					if(output_flag == 1)
					{
						out = open(outputfile,O_WRONLY);
						dup2(out,1);
						close(fds[1]);
						output_flag=0;
					}
		 			else if(x < cmd_count - 1)
		 			{
		 				dup2(fds[1],1);
		 			}
		 		 	if(execvp(temp[0],temp) < 0)
		 			{
		 					printf("Unknown Command\n");
		 			}
		 	}
		 	else if (flag == 1)
		 	{
				signal(SIGCHLD,track);
				flag=0;
				for(i=0;i<100;i++)
				{
					if(p.pid[i] == id)
					{
						p.stat[i] = 0;
						break;
					}
				}
				flag =0;
		 	}
		 	else
		 	{
					 waitpid(id,&status,0);
					 for(i=0;i<100;i++)
					 {
						 if(p.pid[i] == id)
						 {
							 p.stat[i] = 1;
							 break;
						 }
					 }
		 	}
			rfd = fds[0];
		 	close(fds[1]);
		}
		x++;
		}while(temp_1[x] != NULL);
		 cmd_count = 0;
	}
	return 0;
}
