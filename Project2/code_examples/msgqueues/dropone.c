/**
 * @file dropone.c
 * @brief Drops a message into a #defined queue, creating it if user
 *  requested. The message is associated a priority still user
 *  defined. Messages are retrieved in priority order. Higher number
 *  is higher priority. Posix only requires range to be [0,31].
 *
 *  Created by Mij <mij@bitchx.it> on 07/08/05. 
 *  Original source file available on http://mij.oltrelinux.com/devel/unixprg/
 * Modified and annotated by Ramesh Yerraballi
 * Compile with -lrt to link the message queue library
 */
#include <fcntl.h>           /* For O_* constants */
#include <stdio.h>
/* mq_* functions */
#include <mqueue.h>
#include <sys/stat.h>
/* exit() */
#include <stdlib.h>
/* getopt() */
#include <unistd.h>
/* ctime() and time() */
#include <time.h>
/* strlen() */
#include <string.h>


/* name of the POSIX object referencing the queue */
#define MSGQOBJ_NAME    "/myqueue123"
/* max length of a message (just for this process) */
#define MAX_MSG_LEN     70


int main(int argc, char *argv[]) {
    mqd_t msgq_id;
    unsigned int msgprio = 0;
    pid_t my_pid = getpid();
    char msgcontent[MAX_MSG_LEN];
    int create_queue = 0;
    char ch;            /* for getopt() */
    time_t currtime;
    
    
    /* accepting "-q" for "create queue", requesting "-p prio" for message priority */
    while ((ch = getopt(argc, argv, "qp:")) != -1) {
        switch (ch) {
            case 'q':   /* create the queue */
                create_queue = 1;
                break;
            case 'p':   /* specify client id */
                msgprio = (unsigned int)strtol(optarg, (char **)NULL, 10);
                printf("I (%d) will use priority %d\n", my_pid, msgprio);
                break;
            default:
                printf("Usage: %s [-q] -p msg_prio\n", argv[0]);
                exit(1);
        }
    }
    
    /* forcing specification of "-p" argument */
    // Messages are retrieved in the order of their priority so we are forcing it 
    if (msgprio == 0) {
        printf("Usage: %s [-q] -p msg_prio\n", argv[0]);
        exit(1);
    }
    
    /* opening the queue        --  mq_open() */
    //    returns a file_descriptor which is our handle to refer to the queue henceforth
    if (create_queue) {
        /* mq_open() for creating a new queue (using default attributes) */
        // Default attributes: mq_maxmsg <= 10; mq_msgsize <= 8kb
        msgq_id = mq_open(MSGQOBJ_NAME, O_RDWR | O_CREAT | O_EXCL, S_IRWXU | S_IRWXG, NULL);
    } else {
        /* mq_open() for opening an existing queue */
        msgq_id = mq_open(MSGQOBJ_NAME, O_RDWR);
    }
    if (msgq_id == (mqd_t)-1) {
        perror("In mq_open()");
        exit(1);
    }

    /* producing the message: Messages are not numbered so if we want to id a message 
       it is best to put ones pid or some other unique attributed as part of the message */
    currtime = time(NULL);
    snprintf(msgcontent, MAX_MSG_LEN, "Hello from process %u (at %s).", my_pid, ctime(&currtime));
    
    /* sending the message      --  mq_send() */
    // Blocks if the queue is full (default of 10 or set value mq_maxmsg reached)
    mq_send(msgq_id, msgcontent, strlen(msgcontent)+1, msgprio);
    
    /* closing the queue        -- mq_close() */
    // Can perform mq_unlink if we want to delete the queue.
    mq_close(msgq_id);
    
        
    return 0;
}
