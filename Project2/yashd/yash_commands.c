#define _GNU_SOURCE

#include "yash_commands.h"

int MAX_ARGS = 200;
extern char **environ;  // From manpage for exec

/*
    Functions
*/ 
// PCB Related functions
int get_next_jobid(struct PCB *pcb_pointer_head)
{
    int highest_jobid = 0;
    struct PCB *current_pcb = pcb_pointer_head;
    while (current_pcb != NULL)
    {
        if (current_pcb->jobid >= highest_jobid)
        {
            highest_jobid = current_pcb->jobid;
        }
        current_pcb = current_pcb->next_pcb;
    }
    return ++highest_jobid; // Need to get the next id
}

struct PCB* create_job(struct PCB *pcb_pointer_head, char* job_name, int pgid, enum STATE state)
{
    struct PCB *new_job = malloc(sizeof(struct PCB));

    new_job->jobid = get_next_jobid(pcb_pointer_head);
    new_job->most_recent_job = true;
    new_job->process_state = state;
    new_job->name = malloc(sizeof(char) * (strlen(job_name) + 1));
    strcpy(new_job->name, job_name);
    new_job->pgid = pgid;
    new_job->next_pcb = NULL;

    struct PCB *last_job = pcb_pointer_head;
    struct PCB *looper_job = pcb_pointer_head;
    while(looper_job != NULL)
    {
        if (looper_job->most_recent_job)
        {
            looper_job->most_recent_job = false;
        }
        last_job = looper_job;
        looper_job = looper_job->next_pcb;
    }

    if (last_job == NULL)
    {
        return new_job;
    }
    else
    {
        last_job->next_pcb = new_job;
        return pcb_pointer_head;
    }
}

void set_new_latest_jobs(struct PCB *pcb_pointer)
{
    struct PCB *pcb_looper = pcb_pointer;
    while (pcb_looper != NULL)  // Check if there is already a latest job
    {
        if (pcb_looper->most_recent_job)
        {
            return;
        }
        pcb_looper = pcb_looper->next_pcb;
    }
    pcb_looper = pcb_pointer;
    while (pcb_looper != NULL)
    {
        if (pcb_looper->next_pcb == NULL)
        {
            pcb_looper->most_recent_job = true;
        }
        pcb_looper = pcb_looper->next_pcb;
    }
}

char* get_job_status(enum STATE state)
{
    switch(state)
    {
        case STOPPED: return "Stopped";
        case RUNNING: return "Running";
        case DONE: return "Done";
        default: return "";
    }
}

void print_jobs(struct PCB *pcb_pointer_head)
{
    struct PCB *pcb_looper = pcb_pointer_head;
    while (pcb_looper != NULL)
    {
        printf("[%d]%s  %-20s %s\n", pcb_looper->jobid, pcb_looper->most_recent_job ? "+" : "-", get_job_status(pcb_looper->process_state), pcb_looper->name);
        pcb_looper = pcb_looper->next_pcb;
    }
}

char** tokenize_input(char *in_string, int *token_count)
{
    char **ret_tokens;
    char *str_token, *in_string_copy, *saveptr;
    int num_of_tokens = 0;
    int cur_token = 0;

    // Make a copy of the input string, to not ruin the original
    in_string_copy = malloc((strlen(in_string) + 1) * sizeof(char));
    strcpy(in_string_copy, in_string);

    // First pass, get the number of tokens
    str_token = strtok_r(in_string_copy, " ", &saveptr);
    while (str_token != NULL)
    {
        num_of_tokens++;
        str_token = strtok_r(NULL, " ", &saveptr);
    }
    ret_tokens = malloc(sizeof(char*) * (num_of_tokens + 1));

    // Second pass, populate token list
    strcpy(in_string_copy, in_string);
    str_token = strtok_r(in_string_copy, " ", &saveptr);
    while (str_token != NULL)
    {
        ret_tokens[cur_token] = malloc((strlen(str_token) + 1) * sizeof(char));
        strcpy(ret_tokens[cur_token], str_token);
        cur_token++;
        str_token = strtok_r(NULL, " ", &saveptr);
    }

    free(in_string_copy);
    ret_tokens[num_of_tokens] = NULL;
    *token_count = num_of_tokens; // Will just pass token count like this
    return ret_tokens;
}

void signal_handler(int signum)
{
    switch (signum)
    {
        case SIGINT:
            printf("\n# "); // This will setup the prompt again
            break;
        case SIGTSTP:
            printf("\n# "); // This will setup the prompt again
            break;
    }
}

int yash_command(char *cmd_string, struct PCB *pcb_pointer, pid_t *cmd_pgid)
{
    int num_of_tokens;

    signal(SIGINT, signal_handler);
    signal(SIGCHLD, signal_handler);
    signal(SIGTSTP, signal_handler);
    signal(SIGTTOU, SIG_IGN);   // to deal with handing back terminal control to yash
    setpgid(0, 0);

    if (cmd_string != NULL)
    {
        // Commands
        bool command_error = false;
        int cmd1_length = 0;
        int cmd1_tokens = 0;
        int cmd2_length = 0;
        char* command1[MAX_ARGS];
        char* command2[MAX_ARGS];

        // Pipes
        bool pipe_present = false;
        int pipe_token = 0;
        int pipefd[2];

        // Redirection
        int cmd1_redirect_stdin_fd = 0;
        int cmd1_redirect_stdout_fd = 0;
        int cmd2_redirect_stdin_fd = 0;
        int cmd2_redirect_stdout_fd = 0;

        // Process
        int status;
        bool background_process = false;


        // Parse input tokens
        char** input_tokens = tokenize_input(cmd_string, &num_of_tokens);
        cmd1_tokens = num_of_tokens;

        // See if we have one command or two (pipe)
        for (int i = 0; i < num_of_tokens; i++)
        {
            if (strcmp(input_tokens[i], "|") == 0)
            {
                pipe_present = true;
                pipe_token = i;
                cmd1_tokens = pipe_token;   // used to pick out first command when pipe present
            }
        }

        /*
            first/only command parsing
        */
        for (int i = 0; i < cmd1_tokens; i++)
        {
            if (strcmp(input_tokens[i], "<") == 0)  // Redirection: stdin is now from a file (next token)
            {
                if (++i >= cmd1_tokens)
                {
                    printf("yash: syntax error near unexpected token\n");
                    command_error = true;   // FAIL: no filename present
                    break;
                }
                cmd1_redirect_stdin_fd = open(input_tokens[i], O_RDONLY);
                if (cmd1_redirect_stdin_fd == -1)
                {
                    printf("yash: %s: No such file or directory\n", input_tokens[i]);
                    command_error = true;   // FAIL: likely "No such file or directory"
                    break;
                }
            }
            else if (strcmp(input_tokens[i], ">") == 0) // Redirection: stdout is now to a file (next token)
            {
                if (++i >= cmd1_tokens)
                {
                    printf("yash: syntax error near unexpected token\n");
                    command_error = true;   // FAIL: no filename present
                    break;
                }
                cmd1_redirect_stdout_fd = open(input_tokens[i], O_WRONLY|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH);
                if (cmd1_redirect_stdout_fd == -1)
                {
                    printf("yash: %s: No such file or directory\n", input_tokens[i]);
                    command_error = true;   // FAIL: failed to create file (dir permmission problem? no space?)
                    break;
                }
            }
            else if (strcmp(input_tokens[i], "&") == 0)
            {
                background_process = true;  // Don't want this in the command args list
            }
            else
            {
                command1[cmd1_length] = malloc((strlen(input_tokens[i]) + 1) * sizeof(char));
                strcpy(command1[cmd1_length], input_tokens[i]);
                cmd1_length++;
            }
        }
        command1[cmd1_length] = NULL;   // NULL terminate the list

        if (command_error || cmd1_length == 0)
        {
            return 0;   // Skip when no commands entered
        }

        /*
            second command parsing (only if pipe present)
        */
        if (pipe_present)
        {
            for (int i = pipe_token + 1; i < num_of_tokens; i++)
            {
                // Everything up to the pipe
                if (strcmp(input_tokens[i], "<") == 0)  // Redirection: stdin is now from a file (next token)
                {
                    if (++i >= num_of_tokens)
                    {
                        printf("yash: syntax error near unexpected token\n");
                        command_error = true;   // FAIL: no filename present
                        break;
                    }
                    cmd2_redirect_stdin_fd = open(input_tokens[i], O_RDONLY);
                    if (cmd2_redirect_stdin_fd == -1)
                    {
                        printf("yash: %s: No such file or directory\n", input_tokens[i]);
                        command_error = true;   // FAIL: likely "No such file or directory"
                        break;
                    }
                }
                else if (strcmp(input_tokens[i], ">") == 0) // Redirection: stdout is now to a file (next token)
                {
                    if (++i >= num_of_tokens)
                    {
                        printf("yash: syntax error near unexpected token\n");
                        command_error = true;   // FAIL: no filename present
                        break;
                    }
                    cmd2_redirect_stdout_fd = open(input_tokens[i], O_WRONLY|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH);
                    if (cmd2_redirect_stdout_fd == -1)
                    {
                        printf("yash: %s: No such file or directory\n", input_tokens[i]);
                        command_error = true;   // FAIL: failed to create file (dir permmission problem? no space?)
                        break;
                    }
                }
                else
                {
                    command2[cmd2_length] = malloc((strlen(input_tokens[i]) + 1) * sizeof(char));
                    strcpy(command2[cmd2_length], input_tokens[i]);
                    cmd2_length++;
                }
            }
            command2[cmd2_length] = NULL;   // NULL terminate the list
        }

        if (command_error)
        {
            return 0;
        }

        /*
            check if any background processes are completed
        */
        struct PCB *pcb_looper_last = NULL;
        struct PCB *pcb_looper = pcb_pointer;
        while (pcb_looper != NULL)
        {
            int wait_ret = waitpid(-(pcb_looper->pgid), &status, WNOHANG);   // Just check this process
            if (wait_ret == pcb_looper->pgid)
            {
                if (WIFEXITED(status))
                {
                    printf("[%d]%s  %-20s %s\n", pcb_looper->jobid, pcb_looper->most_recent_job ? "+" : "-", "Done", pcb_looper->name);
                    if (pcb_looper_last == NULL)    // First element
                    {
                        pcb_pointer = pcb_pointer->next_pcb;
                        pcb_looper = pcb_pointer;
                    }
                    else
                    {
                        pcb_looper_last->next_pcb = pcb_looper->next_pcb;
                        pcb_looper = pcb_looper->next_pcb;
                    }
                }
            }
            else
            {
                pcb_looper_last = pcb_looper;
                pcb_looper = pcb_looper->next_pcb;
            }
        }
        set_new_latest_jobs(pcb_pointer);   // Will only set if there isn't already one


        /*
            run commands
        */
        if (pipe_present)   // If a pipe is needed, create it
        {
            if (pipe(pipefd) == -1)
            {
                return 0;
            }
        }

        if (strcmp(command1[0], "jobs") == 0)
        {
            print_jobs(pcb_pointer);
            return 0;
        }
        else if (strcmp(command1[0], "fg") == 0)
        {
            // Send SIGCONT to most recent background/stopped process
            struct PCB *pcb_looper_last = NULL;
            struct PCB *pcb_looper = pcb_pointer;
            while (pcb_looper != NULL)
            {
                if (pcb_looper->most_recent_job)
                {
                    if (pcb_looper_last == NULL)    // First element
                    {
                        pcb_pointer = pcb_pointer->next_pcb;
                    }
                    else
                    {
                        pcb_looper_last->next_pcb = pcb_looper->next_pcb;
                    }
                    set_new_latest_jobs(pcb_pointer);   // Reset + in jobs list
                    break;
                }
                pcb_looper_last = pcb_looper;
                pcb_looper = pcb_looper->next_pcb;
            }
            if (pcb_looper == NULL)
            {
                printf("yash: fg: no such job\n");
                return 0; // No most recent job found (bug?)
            }

            printf("%s\n", pcb_looper->name);  // Print job command
            kill(-(pcb_looper->pgid), SIGCONT);
            tcsetpgrp(STDIN_FILENO, pcb_looper->pgid);
            waitpid(-(pcb_looper->pgid), &status, WUNTRACED);
            if (WIFSTOPPED(status))
            {
                pcb_pointer = create_job(pcb_pointer, pcb_looper->name, pcb_looper->pgid, STOPPED);
            }
            tcsetpgrp(STDIN_FILENO, getpgid(getpid()));
            return 0;
        }
        else if (strcmp(command1[0], "bg") == 0)
        {
            struct PCB *pcb_looper = pcb_pointer;
            int pgid_latest_stopped = -1;
            while (pcb_looper != NULL)
            {
                if (pcb_looper->process_state == STOPPED)
                {
                    pgid_latest_stopped = pcb_looper->pgid;
                    break;
                }
                pcb_looper = pcb_looper->next_pcb;
            }

            if (pgid_latest_stopped == -1)  // No jobs to background
            {
                printf("yash: bg: no such job\n");
                return 0;
            }

            pcb_looper = pcb_pointer;
            while (pcb_looper != NULL)
            {
                if (pcb_looper->pgid == pgid_latest_stopped)
                {
                    pcb_looper->process_state = RUNNING;
                    printf("[%d]%s %s &\n", pcb_looper->jobid, pcb_looper->most_recent_job ? "+" : "-", pcb_looper->name);
                    break;
                }
                pcb_looper = pcb_looper->next_pcb;
            }

            kill(-pgid_latest_stopped, SIGCONT);
            return 0;
        }

        // Placeholder process IDs and such
        pid_t cmd1_pid;
        pid_t cmd2_pid;
        int exec_err = 0;
        cmd1_pid = fork();
        if (cmd1_pid == 0)
        {
            if (cmd1_redirect_stdout_fd > 0)
            {
                dup2(cmd1_redirect_stdout_fd, STDOUT_FILENO);   // Handle errors?
            }
            else if (pipe_present)
            {
                dup2(pipefd[1], STDOUT_FILENO);     // Handle errors?
                close(pipefd[0]);
            }

            if (cmd1_redirect_stdin_fd > 0)
            {
                dup2(cmd1_redirect_stdin_fd, STDIN_FILENO);     // Handle errors?
            }

            if (execvpe(command1[0], command1, environ))
            {
                exit(0);    // Bad command
            }
        }
        setpgid(cmd1_pid, 0);

        if (pipe_present)
        {
            cmd2_pid = fork();
            if (cmd2_pid == 0)
            {
                if (cmd2_redirect_stdout_fd > 0)
                {
                    dup2(cmd2_redirect_stdout_fd, STDOUT_FILENO);   // Handle errors?
                }

                if (cmd2_redirect_stdin_fd > 0)
                {
                    dup2(cmd2_redirect_stdin_fd, STDIN_FILENO);     // Handle errors?
                }
                else
                {
                    dup2(pipefd[0], STDIN_FILENO);  // Handle errors?
                    close(pipefd[1]);
                }
                

                if (execvpe(command2[0], command2, environ))
                {
                    exit(0);    // Bad command
                }
            }
            setpgid(cmd2_pid, getpgid(cmd1_pid));
        }

        if (pipe_present)
        {
            close(pipefd[0]);
            close(pipefd[1]);
        }

        if (background_process)
        {
            pcb_pointer = create_job(pcb_pointer, cmd_string, getpgid(cmd1_pid), RUNNING);
        }

        *cmd_pgid = cmd1_pid;
    }
}

// TODO: deal with background jobs and send back the prompt
bool yash_wait(char* cmd_string, struct PCB *pcb_pointer, pid_t cmd1_pid)
{
    // Setup our wait conditions
    // --------------- ORIGINAL YASH CODE BELOW ---------------
    //if (background_process)     // Background job
    //{
    //    /*
    //        In bash, if you do a "ls -l &" it might write out the "Done" before prompting again.
    //        The only way to guarantee this behavior in yash is to sleep before the waitpid call
    //        to give time for the child process to run.
    //    */
    //    pcb_pointer = create_job(pcb_pointer, cmd_string, getpgid(cmd1_pid), RUNNING);
    //    if (waitpid(-getpgid(cmd1_pid), &status, WNOHANG) == getpgid(cmd1_pid)) // This will likely never be valid
    //    {
    //        if (WIFEXITED(status))
    //        {
    //            struct PCB *pcb_looper_last = NULL;
    //            struct PCB *pcb_looper = pcb_pointer;
    //            while (pcb_looper != NULL)
    //            {
    //                if (getpgid(cmd1_pid) == pcb_looper->pgid)
    //                {
    //                    printf("[%d]%s  %-20s %s\n", pcb_looper->jobid, pcb_looper->most_recent_job ? "+" : "-", "Done", pcb_looper->name);
    //                    if (pcb_looper_last == NULL)    // First element
    //                    {
    //                        pcb_pointer = pcb_pointer->next_pcb;
    //                    }
    //                    else
    //                    {
    //                        pcb_looper_last->next_pcb = pcb_looper->next_pcb;
    //                    }
    //                    set_new_latest_jobs(pcb_pointer);
    //                }
    //                pcb_looper_last = pcb_looper;
    //                pcb_looper = pcb_looper->next_pcb;
    //            }
    //        }
    //        else if (WIFSTOPPED(status))
    //        {
    //            pcb_pointer = create_job(pcb_pointer, cmd_string, cmd1_pid, STOPPED);
    //        }
    //    }
    //}
    //else    // Interactive job
    //{
    //    //tcsetpgrp(STDIN_FILENO, cmd1_pid);
    //    waitpid(-cmd1_pid, &status, WUNTRACED);
    //    if (WIFSTOPPED(status))
    //    {
    //        pcb_pointer = create_job(pcb_pointer, cmd_string, cmd1_pid, STOPPED);
    //        kill(tcgetpgrp(STDIN_FILENO), SIGTSTP);
    //    }
    //    if (pipe_present)
    //    {
    //        waitpid(-cmd1_pid, &status, WUNTRACED);
    //    }
    //    //tcsetpgrp(STDIN_FILENO, getpgid(getpid()));
    //}
    int status;
    if (waitpid(-cmd1_pid, &status, WNOHANG) == getpgid(cmd1_pid))
    {
        // SOMETHING IS DONE
        printf("PROCESS FINISHED!\n");
        return true;
    }
}
