#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>

#define MIN_ARGS 3
#define MAX_ARGS 2
#define SERVER_ARG_IDX 1
#define PORT_ARG_IDX 2

#define USAGE_STRING "usage: %s <server address> <port>\n"

void validate_arguments(int argc, char *argv[])
{   // expecint program name, server, port #
    if (argc != 3)
        {
            perror("incorrect arg count\n");
            exit(EXIT_FAILURE);
        }
}

void send_request(int fd) {
    char *line = NULL;
    size_t size;
    ssize_t num;

    // read request from stdin
    printf("Enter your request: \n");
    num = getline(&line, &size, stdin);
    if (num > 0) {
        write(fd, line, num);
    }

    // buffer for server response and count for bytes being read
    char response[4096];
    int totalBytesRead = 0;
    int bytesRead = 0;

    // read response in a loop
    // keeps adding on to end of the response buffer (starting from the position of totalBytes
    while ((bytesRead = read(fd, response + totalBytesRead, sizeof(response))) > 0) {
        totalBytesRead += bytesRead;
    }
    if (bytesRead < 0) {
        perror("read from server failed");
    } else {
        // null terminate the response and print it
        response[totalBytesRead] = '\0';
        printf("%s\n", response);
    }
    free(line);
}


int connect_to_server(struct hostent *host_entry, int port)
{
   int fd;
   struct sockaddr_in their_addr;

   if ((fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
   {
      return -1;
   }
   
   their_addr.sin_family = AF_INET;
   their_addr.sin_port = htons(port);
   their_addr.sin_addr = *((struct in_addr *)host_entry->h_addr);

   if (connect(fd, (struct sockaddr *)&their_addr, sizeof(struct sockaddr)) == -1)
   {
      close(fd);
      perror("connect");
      return -1;
   }
   return fd;
}

struct hostent *gethost(char *hostname)
{
   struct hostent *he;

   if ((he = gethostbyname(hostname)) == NULL)
   {
      herror(hostname);
   }

   return he;
}

int main(int argc, char *argv[])
{
   validate_arguments(argc, argv);
   int port = atoi(argv[PORT_ARG_IDX]);
   if(port < 1024 || port > 65535){
    perror("error invalid port");
    exit(EXIT_FAILURE);
   }
    struct hostent *host_entry = gethost(argv[SERVER_ARG_IDX]);
    if (host_entry) {
        int fd = connect_to_server(host_entry, port);
        if (fd != -1) {
            send_request(fd);
            close(fd);
        }
    }
    return 0;
} 