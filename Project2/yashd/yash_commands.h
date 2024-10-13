#include <fcntl.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

/*
    Definitions, Structs, Enums
*/
enum STATE {
    STOPPED,
    RUNNING,
    DONE
};

struct PCB {
    int jobid;
    bool most_recent_job;   // Used by fg
    enum STATE process_state;
    char* name;
    int pgid;
    struct PCB *next_pcb;
};

/*
    Returns the jobid number that should be assigned
    to a new job. (Next highest number)
*/
int get_next_jobid(struct PCB *pcb_pointer_head);

/*
    Will accept information relevant to a job (name, pgid, state)
    and add a new job entry to the linked-list of jobs.
    (If the list was empty, create the first entry)
*/
struct PCB* create_job(struct PCB *pcb_pointer_head, char* job_name, int pgid, enum STATE state);

/*
    Run after at least 1 job has finished, this will reset
    which job was the most recently run (highest jobid), and
    will be marked with the + after a "jobs" call
*/
void set_new_latest_jobs(struct PCB *pcb_pointer);

/*
    Returns a string representation of the process state
    (Used by the "jobs" command)
*/
char* get_job_status(enum STATE state);

/*
    Will traverse the linked-list of jobs, and print out
    the information as requested.
*/
void print_jobs(struct PCB *pcb_pointer_head);

/*
    Take in the raw input string, and convert it into
    individual tokens, which are returned. Also alter
    the passed in token_count to reflect the number
    of tokens that were returned.
*/
char** tokenize_input(char *in_string, int *token_count);

void signal_handler(int signum);

int yash_command(char *cmd_string, struct PCB *pcb_pointer, pid_t *cmd_pgid);

bool yash_wait(char* cmd_string, struct PCB *pcb_pointer, pid_t cmd1_pid);