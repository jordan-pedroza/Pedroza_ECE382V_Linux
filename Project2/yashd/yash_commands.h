#include <fcntl.h>
#include <readline/readline.h>
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

int get_next_jobid(struct PCB *pcb_pointer_head);

struct PCB* create_job(struct PCB *pcb_pointer_head, char* job_name, int pgid, enum STATE state);

void set_new_latest_jobs(struct PCB *pcb_pointer);

char* get_job_status(enum STATE state);

void print_jobs(struct PCB *pcb_pointer_head);

char** tokenize_input(char *in_string, int *token_count);

void signal_handler(int signum);

int yash_command(char *cmd_string, struct PCB *pcb_pointer);