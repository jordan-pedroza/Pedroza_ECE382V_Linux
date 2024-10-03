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

int main()
{
    int cpid;
    char *inString;
    char **parsedcmd;

    while (inString = readline("# "))
    {
        parsedcmd = parseString(inString);

        cpid = fork();
        if (cpid == 0)
        {
            redirection(parsedcmd);
            execvp(parsedcmd[0], parsedcmd);
        }
        else
        {
            wait((int *)NULL);
        }
    }
}

#define MAX_TOKENS 200
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
    for (int i = 0; args[1] != NULL; i++)
    {
        if (strcmp(args[i], "|") == 0)
        {
            return 1;
        }
    }
    return 0;
}