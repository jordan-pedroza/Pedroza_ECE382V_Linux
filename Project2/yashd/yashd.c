#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <fcntl.h>
#include <errno.h>

#include "yash_common.h"
#include "yash_commands.h"

#define MAX_CLIENTS 10

static char LOG_FILE[] = "/tmp/yashd.log";
static pthread_mutex_t log_lock = PTHREAD_MUTEX_INITIALIZER;
static bool command_in_exec = false;

typedef struct client_thread_args {
    FILE *log_file;
    int client_socket_fd;
    struct sockaddr_in client_sockaddr;
} client_thread_args;

void writeToLog(char* data, char* ip_addr, uint16_t port, FILE *log)
{
    char log_entry[MAX_DATA];
    char date_entry[MAX_DATA];
    time_t cur_time = time(NULL);
    strftime(date_entry, sizeof(date_entry), "%b %d %X", localtime(&cur_time));
    pthread_mutex_lock(&log_lock);
    fprintf(log, "%s yashd[%s:%d]: %s\n", date_entry, ip_addr, port, data);
    fflush(log);
    pthread_mutex_unlock(&log_lock);
}

void reusePort(int s)
{
    int one=1;
    if ( setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (char *) &one, sizeof(one)) == -1 )
	{
	    printf("error in setsockopt\n");
	    exit(-1);
	}
}

// TODO: verify init_daemon content is correct below
void initialize_daemon()
{
    // 1) Make child of init
    int pid = fork();
    if (pid > 0){
        exit(0);
    }

    // 2) Close all file descriptors & redirect to NULL
    int fdtable_size = getdtablesize();
    for (int i = 0; i < fdtable_size; i++)
    {
        close(i);
    }

    int null_fd = open("/dev/null", O_RDWR);
    if (null_fd < 0)
    {
        exit(1);
    }
    dup2(null_fd, STDIN_FILENO);
    dup2(null_fd, STDOUT_FILENO);
    close(null_fd);

    // 3) Put daemon in its own session (own process group?)
    setsid();

    // 4) Move to "safe" working directory
    chdir("/tmp");

    // 5) Ensure only one copy of the daemon is running
    int lock_check_fd = open("/tmp/yashd.pid", O_RDWR | O_CREAT, 0666);
    if (lock_check_fd < 0)
    {
        exit(1);
    }

    if (lockf(lock_check_fd, F_TLOCK, 0) != 0)
    {
        exit(0);
    }
    char pid_str[100];
    sprintf(pid_str, "%d", getpid());
    write(lock_check_fd, pid_str, strlen(pid_str));

    // 6) Set umask
    umask(0);

    // TODO: Any more steps we need? Which order, from example or from class?
}

void *clientThread(void *args)
{
    client_thread_args *arg_data = (client_thread_args *)args;
    char *output_buf = malloc(sizeof(char) * MAX_DATA);
    char *data = malloc(sizeof(char) * MAX_DATA);
    ssize_t rc;
    int num_of_tokens;
    bool process_finished;
    bool yash_command_status;
    // Vars from fork+exec command(s)
    char* cmd_string;
    pid_t cmd_pgid;
    struct PCB *pcb_pointer = malloc(sizeof(struct PCB));   // Create a HEAD job for the job list
    pcb_pointer->jobid = 0;
    bool background_job;
    bool job_cmd;

    clear_string(data, MAX_DATA);
    clear_string(output_buf, MAX_DATA);
    dup2(arg_data->client_socket_fd, STDOUT_FILENO);    // Send STDOUT to socket
    while(1)
    {
        clear_string(data, MAX_DATA);
        // Send prompt (+ response) to client    
        strcpy(data, "\n# ");
        
        rc = send(arg_data->client_socket_fd, data, strlen(data), 0);
        
        if (rc < 0)
        {
            perror("SEND ERROR");
        }
        clear_string(data, MAX_DATA);

        // Get command, signal, plain text from server
        rc = recv(arg_data->client_socket_fd, data, MAX_DATA, 0);
        if (rc < 0)
        {
            perror("RECV ERROR");
            pthread_exit(NULL);
        }
        else if (rc == 0)
        {
            // EOF Return, shutdown connection
            close(arg_data->client_socket_fd);
            pthread_exit(NULL);
        }

        // NULL-terminate client data
        data[rc] = '\0';

        // Remove newline from the input string
        size_t replace_newline_char = strcspn(data, "\n");
        if (replace_newline_char != strlen(data))
        {
            data[replace_newline_char] = 0;
        }
        else
        {
            // Means this was an invalid command? TODO: figure out exception for raw input
            printf("Invalid Command String!\n");
            continue;
        }
        writeToLog(data, inet_ntoa(arg_data->client_sockaddr.sin_addr), ntohs(arg_data->client_sockaddr.sin_port), arg_data->log_file);

        char **tokens = tokenize_input(data, &num_of_tokens);
        char *command_string = malloc(sizeof(char) * MAX_DATA);
        command_string[0] = 0;

        if ((num_of_tokens > 1) && (strcmp(tokens[0], "CMD") == 0))
        {
            for (int i = 1; i < num_of_tokens; i++)
            {
                strcat(command_string, tokens[i]);
                strcat(command_string, " ");
            }
            process_finished = false;
        }
        else if ((num_of_tokens > 1) && (strcmp(tokens[0], "CTL") == 0))
        {
            // TODO: issue kill with relevant signal to current process
            continue;
        }
        background_job = false;
        job_cmd = false;

        //dup2(arg_data->client_socket_fd, STDIN_FILENO);    // Send STDOUT to socket
        //dup2(arg_data->client_socket_fd, STDOUT_FILENO);    // Send STDOUT to socket
        int pfd[2];
        pipe(pfd);
        dup2(pfd[0], STDIN_FILENO);
        yash_command_status = yash_command(command_string, pcb_pointer, &cmd_pgid, &background_job, &job_cmd, pfd[1]);
        // TODO: a case to keep in mind is the wc case (can we just pipe in stuff with a ^D and then wait?)
        while(1)
        {
            //printf("LOOP\n"); fflush(stdout);sleep(1);
            if (!background_job && !job_cmd){
                if (yash_wait(command_string, pcb_pointer, cmd_pgid, background_job))
                {
                    break;
                }
            }
            else
            {
                break;
            }

            // Either try "plain text" mode here, or in other thread, and ensure the socket_fd is global
            clear_string(data, MAX_DATA);
            rc = recv(arg_data->client_socket_fd, data, MAX_DATA, MSG_DONTWAIT);
            if (rc < 0 && (errno == EAGAIN || errno == EWOULDBLOCK))
            {
                // This means nothing is in the buffer
                continue;
            }
            else if (rc < 0)
            {
                perror("RECV ERROR");
                pthread_exit(NULL);
            }
            else if (rc == 0)
            {
                /*printf("TESTING\n");*/ fflush(stdout);
                // EOF Return, shutdown connection
                close(arg_data->client_socket_fd);
                pthread_exit(NULL);
            }
            /*printf("DATA: +%s+\n", data);*/ fflush(stdout);
            //write(pfd[1], data, strlen(data));
            if (strcmp("CTL d\n", data) == 0)
            {
                clear_string(data, MAX_DATA);
                /*printf("TESTING\n");*/ fflush(stdout);
                close(pfd[1]); close(pfd[0]);
                
            }else{write(pfd[1], data, strlen(data));}

            if (send_signal_to_job(cmd_pgid, data))
            {
                // Means we stopped a process with ^Z
                pcb_pointer = create_job(pcb_pointer, command_string, cmd_pgid, STOPPED);
                break;
            }
        }
    }
}

int main(int argc, char* argv[])
{
    struct addrinfo addrinfo_hints;
    struct addrinfo *addrinfo_result;
    struct sockaddr_in server;
    struct sockaddr_in client;

    int server_socket_fd, client_socket_fd, rc, next_thread;
    FILE *log_file;
    pthread_t threads[MAX_CLIENTS];
    client_thread_args thread_args[MAX_CLIENTS];

    // TODO: jobs output not happening when daemon initialized??
    initialize_daemon();  // Just for ease of debug

    addrinfo_hints.ai_family = AF_INET;
    addrinfo_hints.ai_socktype = SOCK_STREAM;
    addrinfo_hints.ai_protocol = 0;
    addrinfo_hints.ai_flags = 0;    // MEGA errors when not 0'ing out everything unused here
    addrinfo_hints.ai_canonname = NULL;
    addrinfo_hints.ai_addr = NULL;
    addrinfo_hints.ai_next = NULL;


    // --- Server IP/Port Step ---
    if (getaddrinfo("localhost", YASHD_PORT, &addrinfo_hints, &addrinfo_result) != 0)
    {
        printf("ERROR IN GETADDRINFO\n");
        exit(-1);    // Problem with getting localhost
    }

    // Expecting only a single result
    if (addrinfo_result == NULL)
    {
        printf("NO ADDRINFO_RESULT\n");
        exit(-1);    // localhost not found?
    }

    char hostname[MAX_DATA];
    char servername[MAX_DATA];
    getnameinfo(addrinfo_result->ai_addr, addrinfo_result->ai_addrlen, hostname, MAX_DATA, servername, MAX_DATA, NI_NUMERICHOST);
    freeaddrinfo(addrinfo_result);
    //printf("HOSTNAME: %s -- SERVERNAME: %s\n", hostname, servername);

    // --- Server Socket Step ---
    server_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket_fd < 0)
    {
        perror("NO SOCKET");
        exit(-1);
    }
    reusePort(server_socket_fd);

    // --- Server Bind Step ---
    server.sin_family = AF_INET;
    server.sin_port = htons(atoi(YASHD_PORT));
    server.sin_addr.s_addr = inet_addr(hostname); // This is just for testing on localhost
    if (bind(server_socket_fd, (struct sockaddr *) &server, sizeof(server)))
    {
        perror("BINDING ERROR");
        close(server_socket_fd);
        exit(-1);
    }

    // --- Server Listen Step
    if (listen(server_socket_fd, MAX_CLIENTS))
    {
        perror("LISTEN ERROR");
        exit(-1);
    }

    log_file = fopen(LOG_FILE, "w");
    next_thread = 0;
    while (1)
    {
        // --- Server Accept Step
        socklen_t client_size = sizeof(client);
        client_socket_fd = accept(server_socket_fd, (struct sockaddr *) &client, &client_size);
        if (client_socket_fd < 0)
        {
            perror("ACCEPT ERROR");
            //exit(-1);
        }

        thread_args[next_thread].client_socket_fd = client_socket_fd;
        thread_args[next_thread].client_sockaddr = client;
        thread_args[next_thread].log_file = log_file;
        rc = pthread_create(&threads[next_thread], NULL, clientThread, &thread_args[next_thread]);
        if (rc)
        {
            printf("ERROR WITH PTHREAD_CREATE!\n");
            exit(-1);
        }
        next_thread++;  // TODO: find a different way to deal with connection limit (10; not sure it'll be tested?)

    }
}
