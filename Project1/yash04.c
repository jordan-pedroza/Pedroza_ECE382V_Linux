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
#include <termios.h> 
#include <stdbool.h>

#define MAX_JOBS 20

char **parseString(char *input_string); // parses an command into strings
void redirection(char **args);
int pipe_detector(char **args);
int ampersand_detector(char **args);
int jobs_detector(char **args);
int fg_detector(char **args);
int bg_detector(char **args);
void sigtstp_handler(int sig);
void sigint_handler(int sig);
void init_mystruct(void);
void printjobslist(void);

int cpid = -1;
int is_bg;
int is_sigtstp = 0;
int job_counter = 0;
int job_index = 0;

typedef struct
{
    int job_id; //start at [1]
    pid_t jpid;// internally accessed
    char plus_or_minus; // - +
    char *status; // Running/Stopped/Done
    char *command; // sleep 25
} job_struct;

 static job_struct jobs[MAX_JOBS];



int main()
{
    int cpid = -1;
    char *inString;
    char *commandname;
    char **parsedcmd;
    int status;

    init_mystruct();
    signal(SIGINT, sigint_handler);   // Handle Ctrl+C
    signal(SIGTSTP, sigtstp_handler); // Handle Ctrl+Z
    

    while (inString = readline("# "))
    {
        commandname = inString;
        parsedcmd = parseString(inString);
        

        if (jobs_detector(parsedcmd))
        {
            //print my jobs list func
            printjobslist();
        }

        if (ampersand_detector(parsedcmd))
        {
            //printf("%s \n", "&");
        }

        if (fg_detector(parsedcmd))
        {
            //changetofg();
        }

        if (bg_detector(parsedcmd))
        {

            //changetobg();
        }

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
        else 
        {
            cpid = fork();
            //printf("\n%s cpid = %d\n", "Inside of child", cpid);
            if (cpid == 0)
            {
                //setpgid(0,0);
                redirection(parsedcmd);
                execvp(parsedcmd[0], parsedcmd);
            }
            else
            {
                //printf("%s \n", "stuck sucess" );
                //setpgid(cpid,0);
                if (job_counter > 0){
                    jobs[job_index].plus_or_minus = '-';
                    job_index ++;
                }

                jobs[job_index].plus_or_minus = '+';
                jobs[job_index].jpid = -cpid;
                jobs[job_index].status = "Running";
                jobs[job_index].command = commandname;

                printf("Job ID\tPID\tStatus\t\tCommand\n");
                printf("-------------------------------------------------\n");
                printf("%d[%c]\t%d\t%s\t\t%s\n", 
                jobs[job_index].job_id + 1, 
                jobs[job_index].plus_or_minus,
                jobs[job_index].jpid, 
                jobs[job_index].status, 
                jobs[job_index].command);

                //printf("\n%s cpid = %d\n", "what is my cpid", cpid);
                waitpid(cpid, &status, WUNTRACED | WCONTINUED);
                //printf("\n%s cpid = %d\n", "what is my cpid", cpid);



                //printf("%d",status);

                /*if(WIFEXITED(status) == EXIT_FAILURE)
                {
                    printf("you suck");
                }*/
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

int jobs_detector(char **args)
{
    for (int i = 0; args[i] != NULL; i++)
    {
        if (strcmp(args[i], "jobs") == 0)
        {
            return 1;  // Jobs detected
        }
    }
    return 0;  // No jobs found
}

int fg_detector(char **args)
{
    for (int i = 0; args[i] != NULL; i++)
    {
        if (strcmp(args[i], "fg") == 0)
        {
            return 1;  // A fg detected
        }
    }
    return 0;  // No fg found
}

int bg_detector(char **args)
{
    for (int i = 0; args[i] != NULL; i++)
    {
        if (strcmp(args[i], "bg") == 0)
        {
            return 1;  // A bg detected
        }
    }
    return 0;  // No bg found
}

int ampersand_detector(char **args)
{
    for (int i = 0; args[i] != NULL; i++)
    {
        if (strcmp(args[i], "&") == 0)
        {
            return 1;  // Ampersand detected
        }
    }
    return 0;  // No ampersand found
}

// Signal handler for Ctrl+C
void sigint_handler(int sig) {
    //printf("\n%s cpid = %d\n", "Inside of ^C", cpid);
    if ((cpid == 0)) {
        // Send SIGINT to the child process group
        //printf("\n%s cpid = %d\n", "Inside of if", cpid);
        kill(cpid, SIGINT);
        printf("\nProcess terminated.\n");
        sleep(2);
        // cpid = -1;
    }
}

// Signal handler for Ctrl+Z
void sigtstp_handler(int sig) {
    //printf("\n%s cpid = %d\n", "Inside of ^Z", cpid);
    if ((cpid != -1)) {
        for(int i = 0; i < job_counter; i ++){
            if (jobs[i].jpid == cpid) {
                strcpy(jobs[i].status, "Stopped");
            }
        }
        //printf("\n%s cpid = %d\n", "Inside of ^Z", cpid);
        // Send SIGTSTP to the child process group
        printf("\nProcess stopped.\n");
        kill(cpid, SIGTSTP);
        sleep(2);
        //cpid = -1;
    }
}

void init_mystruct(void)
{
    for (int i = 0; i <= MAX_JOBS; i++)
    {
        job_struct NewJob = {
            .job_id = i + 1,
            .plus_or_minus = '-',
            .jpid = 0,
            .status = "Done",
            .command = "Name PlaceHolder"
        };
    }
}


void printjobslist(void){
    printf("Job ID\tPID\tStatus\t\tCommand\n");
    printf("-------------------------------------------------\n");
    for (int i = 0; i < job_counter; i++)
    {
        printf("%d[%c]\t%d\t%s\t\t%s\n", 
        jobs[job_index].job_id, 
        jobs[job_index].plus_or_minus,
        jobs[job_index].jpid, 
        jobs[job_index].status, 
        jobs[job_index].command);
    }
}