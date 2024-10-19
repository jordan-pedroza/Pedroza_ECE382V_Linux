#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <pthread.h>
#include <stdbool.h>

#include "yash_common.h"

void sigtstp_handler(int sig);
void sigint_handler(int sig);
void *comms_thread(void *arg);
char signal_message[MAX_DATA];
int server_socket_fd;
int rc;
bool prompt_flag;

//////////////////////////////////////////
//prototype for the communication thread//
//////////////////////////////////////////

int main(int argc, char *argv[])
{
    struct addrinfo addrinfo_hints;
    struct addrinfo *addrinfo_result;
    struct sockaddr_in server;
    struct sockaddr_in client;
    
    pthread_t comm_thread;
    char terminal_input_string[MAX_DATA];
    char *data = malloc(sizeof(char) * MAX_DATA);

    addrinfo_hints.ai_family = AF_INET;
    addrinfo_hints.ai_socktype = SOCK_STREAM;
    addrinfo_hints.ai_protocol = 0;
    addrinfo_hints.ai_flags = 0; // MEGA errors when not 0'ing out everything unused here
    addrinfo_hints.ai_canonname = NULL;
    addrinfo_hints.ai_addr = NULL;
    addrinfo_hints.ai_next = NULL;

    signal(SIGINT, sigint_handler);   // Handle Ctrl+C
    signal(SIGTSTP, sigtstp_handler); // Handle Ctrl+Z

    if (argc < 2)
    {
        printf("ERROR: Missing IP Address!\n");
        exit(-1);
    }

    // --- Client IP/Port Step ---
    rc = getaddrinfo(argv[1], YASHD_PORT, &addrinfo_hints, &addrinfo_result);
    if (rc)
    {
        printf("ERROR IN GETADDRINFO: %s\n", gai_strerror(rc));
        exit(-1); // Problem with getting server address
    }

    // --- Client Socket IP/Port Step ---
    server_socket_fd = socket(AF_INET, SOCK_STREAM, 0);

    // --- Client Connect Step ---
    struct sockaddr_in *sockaddr_result;
    sockaddr_result = (struct sockaddr_in *)addrinfo_result->ai_addr;
    server.sin_family = AF_INET;
    server.sin_port = htons(atoi(YASHD_PORT));
    server.sin_addr.s_addr = sockaddr_result->sin_addr.s_addr;

    if (connect(server_socket_fd, (struct sockaddr *)&server, sizeof(server)))
    {
        close(server_socket_fd);
        perror("CONNECT ERROR");
        exit(0);
    }

    if (pthread_create(&comm_thread, NULL, comms_thread, NULL) != 0) {
        perror("Failed to create communication thread");
        exit(-1);
        }  

    while (1)
    {
        clear_string(data, MAX_DATA);
        memset(signal_message, 0, sizeof(signal_message));

        // ^C and ^Z get stuck here, while the server sends a new prompt
        if (fgets(terminal_input_string, sizeof(terminal_input_string), stdin) == NULL)
        {
             if (feof(stdin)) {
                clearerr(stdin);  // Clear EOF so we can continue reading from stdin
            }
            if (prompt_flag)
            {
                printf("\n");            // Reset terminal line after closing yash
                strcpy(data, "CTL d\n"); // this is the string contructor for the CMD value
                rc = send(server_socket_fd, data, strlen(data), 0);
                if (rc < 0)
                {
                    perror("SEND ERROR");
                    exit(-1);
                }

                close(server_socket_fd);
                exit(0);
            }
            else
            {
                //printf("we are back in the ctl d statement");
                
                //printf("\n"); // Reset terminal line after closing yash
                strcpy(data, "CTL d\n");    // this is the string contructor for the CMD value
                rc = send(server_socket_fd, data, strlen(data), 0);
                clear_string(data, MAX_DATA);
                //sleep (1);
                if (rc < 0)
                {
                    perror("SEND ERROR");
                    exit(-1);
                }
                prompt_flag = true;
            }
        }
        // -----------------------------------------------------------------
        

        // -------------------------------------------------------------------

        if (prompt_flag)
        {
            strcpy(data, "CMD "); // this is the string contructor for the CMD value
            strcat(data, terminal_input_string);
            strcat(data, "\n");
        }
        else
        {
            strcat(data, terminal_input_string);
        }

        // Send command, signal, plain text to server
        //printf("running the loop");
        rc = send(server_socket_fd, data, strlen(data), 0);
        if (rc < 0)
        {
            perror("SEND ERROR");
            exit(-1);
        }
        prompt_flag = false;
    }
    pthread_join(comm_thread, NULL);
    free(data);
}

// Signal handler for Ctrl+C
void sigint_handler(int sig)
{
    snprintf(signal_message, MAX_DATA, "CTL c\n");
    rc = send(server_socket_fd, signal_message, strlen(signal_message), 0);
    if (rc < 0)
    {
        perror("SEND ERROR");
        exit(-1);
    }
    // Clear the signal_message buffer after sending
    memset(signal_message, 0, sizeof(signal_message));
}
// Signal handler for Ctrl+Z
void sigtstp_handler(int sig)
{
    snprintf(signal_message, MAX_DATA, "CTL z\n");
    rc = send(server_socket_fd, signal_message, strlen(signal_message), 0);
    if (rc < 0)
    {
        perror("SEND ERROR");
        exit(-1);
    }
    // Clear the signal_message buffer after sending
    memset(signal_message, 0, sizeof(signal_message));
}

void *comms_thread(void *arg)
{
    char buffer[MAX_DATA];
    int rc;

    while (1)
    {
        // Clear the buffer for each message
        memset(buffer, 0, MAX_DATA);

        // Wait for data from the server
        //printf("MSGMSGMSG");
        rc = recv(server_socket_fd, buffer, MAX_DATA, 0);
        if (rc < 0)
        {
            perror("RECV ERROR");
            pthread_exit(NULL);
        }
        else if (rc == 0)
        {
            // Server has closed the connection
            close(server_socket_fd);
            pthread_exit(NULL); // Exit the thread
        }
        buffer[rc] = 0;

        // Print the message from the server to stdout
        printf("%s", buffer);
        fflush(stdout);

        // Check last 3 chars out of the buffer
        int str_cmp_start = strlen(buffer) - 3;
        char test[50];
        strncpy(test, buffer + str_cmp_start, 4);
        if (strcmp(test, "\n# ") == 0)
        {
            // Getting either one or two data packets out of recv (prompt + result of exec)
            prompt_flag = true;
        }
    }
    return NULL;
}
