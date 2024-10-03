#include <stdio.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/stat.h>

char **parseString(char *input_string); // parses an command into strings
void redirection(char **args);
int pipe_detector(char **args);

int main()
{
    int cpid;
    char *inString;
    char **parsedcmd;

    while (inString = readline("# "))
    {
        parsedcmd = parseString(inString);
        
        //printf("%s \n", "parsed sucess" );
        
        if (pipe_detector(parsedcmd))
        {   
            //printf("%s \n", "pipe detector" );
            char cmdstrpipe1[2048] = {'\0'};
            char cmdstrpipe2[2048] = {'\0'};

            int pc1pid;
            int pc2pid;

            int i;
            int j;
            int k;

            for (i = 0; parsedcmd[i] != NULL; i++)
            {
                if (strcmp(parsedcmd[i], "|") == 0)
                {
                    break;
                }
            }
            for (j = 0; j < i; j++)
            {
                strcat(cmdstrpipe1, parsedcmd[j]);
                strcat(cmdstrpipe1, " ");
            }
            for (k = i + 1; parsedcmd[k] != NULL; k++)
            {
                strcat(cmdstrpipe2, parsedcmd[k]);
                strcat(cmdstrpipe2, " ");
            }
            //printf("%s \n", cmdstrpipe1);
            //printf("%s \n", cmdstrpipe2);

            if (cmdstrpipe1[strlen(cmdstrpipe1) -1] == ' ')
            {
                //printf("%s \n", "we did this 1");
                cmdstrpipe1[strlen(cmdstrpipe1) -1] = '\0';
            }

            if (cmdstrpipe2[strlen(cmdstrpipe2) -1] == ' ')
            {
                
                //printf("%s \n", "we did this 1");
                cmdstrpipe2[strlen(cmdstrpipe2) -1] = '\0';
            }
            
            //printf("%s \n", "pipe about to make" );

            int pfd[2];
            pipe(pfd);
            
           // printf("%s \n", "fork 1 start" );
            pc1pid = fork();

            if (pc1pid == 0)
            {
                //printf("%s \n", "fork 1 inside" );
                dup2(pfd[1], STDOUT_FILENO);
                close(pfd[0]);
                close(pfd[1]);
                char **parsedcmdpipe1;
                parsedcmdpipe1 = parseString(cmdstrpipe1);
                redirection(parsedcmdpipe1);
                execvp(parsedcmdpipe1[0], parsedcmdpipe1);
                //printf("%s \n", "fork 1 finish" );
            }

            //printf("%s \n", "fork 2 start" );
            pc2pid = fork();
            if (pc2pid == 0)
            {
                // printf("%s \n", "fork 2 inside" );
                dup2(pfd[0], STDIN_FILENO);
                close(pfd[0]);
                close(pfd[1]);
                char **parsedcmdpipe2;
                //printf("%s \n", "fork 2 " );
                parsedcmdpipe2 = parseString(cmdstrpipe2);
                //printf("%s \n", "fork 2 parse" );
                redirection(parsedcmdpipe2);
                //printf("%s \n", "fork 2 redir" );
                execvp(parsedcmdpipe2[0], parsedcmdpipe2);
                //printf("%s \n", "fork 2 exec" );
            }
            //printf("%s \n", " we out");
            close(pfd[0]);
            close(pfd[1]);
            waitpid(pc1pid, NULL, 0);
            waitpid(pc2pid, NULL, 0);

            // look up wait pid function
        }
       
        // signal handler first 
        //then jobs
        else 
        {
            //printf("%s \n", " we stuck in the reg fork why are we here");
            cpid = fork();
            if (cpid == 0)
            {
                redirection(parsedcmd);
                execvp(parsedcmd[0], parsedcmd);
            }
            else
            {
                //printf("%s \n", "stuck sucess" );
                wait((int *)NULL);
            }
        }
        
    }
}

#define MAX_TOKENS 2048
// parseString function (read the man page for strtok)
char **parseString(char *input)
{
    char **tokens = malloc(MAX_TOKENS * sizeof(char *)); // Allocate space for tokens
    char *token;
    int i = 0;

    // Get the first token (split by space in this case)
    token = strtok(input, " ");

    // Loop through the string and get the next tokens
    while (token != NULL)
    {
        tokens[i++] = token;       // Store the token in the array
        token = strtok(NULL, " "); // Continue tokenizing the rest of the string
    }

    tokens[i] = NULL; // Mark the end of the array with NULL (important for iteration)

    return tokens; // Return the array of tokens
}

void redirection(char **args)
{

    int input, output;
    /*INPUT REDIR*/
    for (int i = 0; args[i] != NULL; i++)
    {

        if (strcmp(args[i], "<") == 0)
        {
            input = open(args[i + 1], O_RDONLY);
            if (input < 0)
            {
                printf("Open File Failed `DOES NOT EXIST`: %s\n", args[i + 1]);
                exit(EXIT_FAILURE);
            }
            dup2(input, STDIN_FILENO);
            close(input);
            args[i] = NULL;
            break;
        }
    }
    /*OUTPUT REDIR*/
    for (int i = 0; args[i] != NULL; i++)
    {
        if (strcmp(args[i], ">") == 0)
        {
            output = open(args[i + 1], O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
            dup2(output, STDOUT_FILENO);
            close(output);
            args[i] = NULL;
            break;
        }
    }
}

int pipe_detector(char **args)
{
    //printf("%s \n", "pipe fail" );
    
    for (int i = 0; args[i] != NULL; i++)
    {
        if (strcmp(args[i], "|") == 0)
        {
            return 1;
        }
    }
    return 0;
}