#include <stdio.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>


char ** parseString(char * input_string); //parses an command into strings 

int main(){
    int cpid;
    char *inString;
    char **parsedcmd;

    while(inString = readline("# ")){

	parsedcmd = parseString(inString);
	/* check and see if theres a pipe or pipe 
	then send to other functions and make good (change std in out or err)*/
	cpid = fork();
	if (cpid == 0){
	    execvp(parsedcmd[0],parsedcmd);
	}else{
	    wait((int *)NULL);
	}
    }
}

#define MAX_TOKENS 200
// parseString function (read the man page for strtok)
char **parseString(char* input) {
    char **tokens = malloc(MAX_TOKENS * sizeof(char*));  // Allocate space for tokens
    char *token;
    int i = 0;

    // Get the first token (split by space in this case)
    token = strtok(input, " ");
    
    // Loop through the string and get the next tokens
    while (token != NULL) {
        tokens[i++] = token;   // Store the token in the array
        token = strtok(NULL, " "); // Continue tokenizing the rest of the string
    }

    tokens[i] = NULL; // Mark the end of the array with NULL (important for iteration)

    return tokens;  // Return the array of tokens
}
