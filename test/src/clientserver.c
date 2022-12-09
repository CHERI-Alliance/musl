#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

#define IP_ADDRESS "127.0.0.1"
#define PORT 12054
#define BUFFER_SIZE 1024

pthread_mutex_t STDOUT_MUTEX;

void *server(void *args)
{
    int server_fd, client_fd;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("Failed to create server socket.");
        return (void *) 1;
    }

    /* Allow server socket to resuse port and address. */
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
    {
        perror("Failed to set server socket options.");
        return (void *) 2;
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    /* Bind the server to localhost and port number. */
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address))<0)
    {
        perror("Failed to bind server socket to port.");
        return (void *) 3;
    }

    /* Listen for 1 client connections on the server. */
    if (listen(server_fd, 1) < 0)
    {
        perror("Failed to listen on port.\n");
        return (void *) 4;
    }

    /* Prevent printf race conditions between server and client thread. */
    pthread_mutex_lock(&STDOUT_MUTEX);

    /* Accept client connection. */
    if ((client_fd = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0)
    {
        perror("Failed to accept client connection.");
        return (void *) 5;
    }

    /* Send message to client connection. */
    char *message = "Morello\n";
    send(client_fd, message, strlen(message), 0);
    printf("Message sent to client: %s", message);

    pthread_mutex_unlock(&STDOUT_MUTEX);

    /* Shutdown client connection and server socket. */
    shutdown(client_fd, SHUT_RDWR);
    close(server_fd);

    return (void *) 0;
}

void *client(void *args)
{
    int sockfd;
    char buffer[BUFFER_SIZE] = {0};
    struct sockaddr_in servaddr;

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Failed to create client socket.");
        return (void *) 1;
    }

    /* Zero server socket information. */
    memset(&servaddr, 0, sizeof(servaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    servaddr.sin_addr.s_addr = inet_addr(IP_ADDRESS);

    /* Connect to server socket. */
    if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
    {
        perror("Failed to connect to the server socket.");
        return (void *) 2;
    }

    pthread_mutex_lock(&STDOUT_MUTEX);

    /* Read message from server. */
    int valread = read(sockfd, buffer, BUFFER_SIZE);
    if(valread == -1)
    {
        perror("Failed to read data from server socket.");
    }

    /* Give time for server to print out first. */
    printf("Message recieved from server: %s", buffer);

    pthread_mutex_unlock(&STDOUT_MUTEX);

    close(sockfd);
    return (void *) 0;
}

int main(int argc, char *argv[])
{
    pthread_t clientThread, serverThread;
    int *clientRes = 0;
    int *serverRes = 0;

    pthread_mutex_init(&STDOUT_MUTEX, NULL);

    pthread_create(&serverThread, NULL, &server, NULL);
    pthread_create(&clientThread, NULL, &client, NULL);

    pthread_join(serverThread, (void *) clientRes);
    pthread_join(clientThread, (void *) serverRes);

    if(serverRes != 0) return *serverRes;
    if(clientRes != 0) return *clientRes;

    return 0;
}
