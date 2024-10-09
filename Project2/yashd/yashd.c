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
#include <pthread.h>

#include "yash_common.h"

static int MAX_CLIENTS = 10;    // TODO: not sure what this should be?
static int MAX_NAME_LENGTH = 100;
static char LOG_FILE[] = "/tmp/yashd.log";

void writeToLog(char* data, char* ip_addr, uint16_t port)
{
    char log_entry[MAX_DATA];
    char date_entry[MAX_DATA];
    time_t cur_time = time(NULL);
    strftime(date_entry, sizeof(date_entry), "%b %d %X", localtime(&cur_time));
    snprintf(log_entry, MAX_DATA * sizeof(char), "%s yashd[%s:%d]: %s", date_entry, ip_addr, port, data);

    printf("DEBUG ENTRY: %s\n", log_entry);
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

typedef struct client_thread_args {
    int client_socket_fd;
    struct sockaddr_in client_sockaddr;
} client_thread_args;

//void clientThread(int client_socket_fd, struct sockaddr_in client_sockaddr)
void *clientThread(void *args)
{
    client_thread_args *arg_data = (client_thread_args *)args;
    // TODO: grab all daemon init stuff
    //char data[MAX_DATA];
    char *data = malloc(sizeof(char) * MAX_DATA);
    ssize_t rc;

    clear_string(data, MAX_DATA);
    while(1)
    {
        // Send prompt (+ response) to client
        strcat(data, "\n# ");
        rc = send(arg_data->client_socket_fd, data, strlen(data), 0);
        if (rc < 0)
        {
            perror("SEND ERROR");
        }
        clear_string(data, MAX_DATA);

        // Get command, signal, plain text from server
        rc = recv(arg_data->client_socket_fd, data, MAX_DATA, 0);
        writeToLog(data, inet_ntoa(arg_data->client_sockaddr.sin_addr), ntohs(arg_data->client_sockaddr.sin_port));
        if (rc < 0)
        {
            perror("RECV ERROR");
            pthread_exit(NULL);
        }
        else if (rc == 0)
        {
            // EOF Return, shutdown connection
            printf("DISCONNECT!\n");
            close(arg_data->client_socket_fd);
            pthread_exit(NULL);
        }

        // NULL-terminate client data
        data[rc] = '\0';
    }
}

int main(int argc, char* argv[])
{
    struct addrinfo addrinfo_hints;
    struct addrinfo *addrinfo_result;
    struct sockaddr_in server;
    struct sockaddr_in client;

    int server_socket_fd, client_socket_fd, rc, next_thread;
    pthread_t threads[MAX_CLIENTS];
    client_thread_args thread_args[MAX_CLIENTS];

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

    char hostname[MAX_NAME_LENGTH];
    char servername[MAX_NAME_LENGTH];
    getnameinfo(addrinfo_result->ai_addr, addrinfo_result->ai_addrlen, hostname, MAX_NAME_LENGTH, servername, MAX_NAME_LENGTH, NI_NUMERICHOST);
    freeaddrinfo(addrinfo_result);
    printf("HOSTNAME: %s -- SERVERNAME: %s\n", hostname, servername);

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
    //server.sin_addr.s_addr = htonl(INADDR_ANY); // REMOVE LATER, NEED TO ACCEPT ALL IP ADDRESSES
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
        printf("CLIENT CONNECTED: %s:%d\n", inet_ntoa(client.sin_addr), ntohs(client.sin_port));

        // ==========================   MOVE TO A PTHREAD_CREATE FUNCTION
        //clientThread(client_socket_fd, client);
        thread_args[next_thread].client_socket_fd = client_socket_fd;
        thread_args[next_thread].client_sockaddr = client;
        rc = pthread_create(&threads[next_thread], NULL, clientThread, &thread_args[next_thread]);
        if (rc)
        {
            printf("ERROR WITH PTHREAD_CREATE!\n");
            exit(-1);
        }
        next_thread++;
        // ==========================

    }

    printf("PAST BIND!\n");
}




//yash code here

//1. grabbing the cmd
//2. if there is a stdoout sending to socket
//2.1 redirection
//2.2 piping 
//3. if there std in 

// where are we sending files in 