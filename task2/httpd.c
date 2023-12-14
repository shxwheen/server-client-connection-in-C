#define _GNU_SOURCE
#include "net.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <signal.h>
#include <sys/wait.h>


void sendResponse(int sockFd, const char *path, int sendContent);
void sendErrorResponse(int sockFd, int code, const char* message);

// signal handling
void sigchld_handler(int s){
   while(waitpid(-1, NULL, WNOHANG) > 0);
}

void sigint_handler(int signum){
    printf("\nReceived SIGINT (signum: %d)\n", signum);
    exit(EXIT_SUCCESS);
}
void handle_request(int nfd)
{

   printf("handling new request\n");

   FILE *network = fdopen(nfd, "r");
   char *line = NULL;
   size_t size;
   ssize_t num;

   // error checking 
   if (network == NULL)
   {
      perror("fdopen");
      close(nfd);
      return;
   }

   // read from client
   while ((num = getline(&line, &size, network)) >= 0)
   {

      printf("received line: %s\n",line);

      // declare variables for http method, path to resource
      char method[10], path[100], protocol[10];
      // extracting input to 3 char[] using sscanf
      if(sscanf(line, "%s %s %s", method, path, protocol) != 3){
         sendErrorResponse(nfd, 400, "Bad Request");
         free(line);
         fclose(network);
         return;
      };
      
      // skip slash
      if(path[0] == '/'){
         memmove(path,path + 1, strlen(path));
      }

      // check request type & send response to client
      if (strcmp(method, "GET") == 0) {
         printf("Method: %s, Path: %s, Protocol: %s\n",method,path,protocol);
         sendResponse(nfd,path,1);
      } else if (strcmp(method, "HEAD") == 0) {
         printf("Method: %s, Path: %s, Protocol: %s\n",method,path,protocol);
         sendResponse(nfd,path,0);
      } else {
         // handle unknown method
         sendErrorResponse(nfd,501,"Not Implemented");
         printf("BS");
         free(line);
         fclose(network);
         
         //fclose(file);
         return;
      }
   }
   if(num <= 0){
      printf("client disconnected\n");
      //system("ps");
   }
   free(line);
   fclose(network);
}


void run_service(int fd)
{
   while (1)
   {
      int nfd = accept_connection(fd);
      if (nfd != -1)
      {
         system("ps");
         printf("Connection established\n");

         pid_t pid = fork();
         if(pid == -1){
            perror("fork");
            close(nfd);
         } else if (pid == 0){
            close(fd);
            handle_request(nfd);
            exit(0);
         } else{
            close(nfd);
         }
      }
   }
}

int main(int argc, char *argv[])
{
   // error check
   if (argc != 2){
      perror("wrong arg count. pls provide only port");
      exit(EXIT_FAILURE);
   }
      // convert input to int
   int port = atoi(argv[1]);
   if(port < 1024 || port > 65535){
      printf("ERROR invalid port number");
      exit(EXIT_FAILURE);
   }

    // signal handler struct
    struct sigaction sa;
    sa.sa_handler = sigchld_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }
    signal(SIGINT,sigint_handler);

   // service
   int fd = create_service(port);
   if (fd == -1)
   {
      perror(0);
      exit(EXIT_FAILURE);
   }

   printf("listening on port: %d\n", port);
   run_service(fd);
   close(fd);
   return 0;
}

void sendResponse(int sockFd, const char *path, int sendContent) {

   // check path of file
    struct stat fileStat;
    if (stat(path, &fileStat) == -1) {
        sendErrorResponse(sockFd, 404, "Not Found");
        return;
    }

    // arrange header
    char header[1024] = "";
    sprintf(header, "HTTP/1.0 200 OK\r\n");
    strcat(header, "Content-Type: text/html\r\n");
    char contentLength[50] = "";
    sprintf(contentLength, "Content-Length: %ld\r\n\r\n", fileStat.st_size);
    strcat(header, contentLength);
    write(sockFd, header, strlen(header));
    printf("Sending headers: %s\n", header);


   // if sendContent flag is high
    if (sendContent) {
        printf("For path: %s\n", path);
        //fprintf(file,"%s",path);

        // open file and error handling
        int fd = open(path, O_RDONLY);
        if (fd == -1) {
            perror("open");
            close(sockFd);
            return;
        }
         // for the separate html file output
        char filename[100];
        pid_t pid = getpid();
        sprintf(filename,"tests/client: %d.html",pid);
        FILE *file = fopen(filename,"w");
        if(file == NULL){
        perror("fopen");
        exit(EXIT_FAILURE);
      }


         // buffer to store data from file
         // track total bytesRead
        char buffer[4096] = "";
        ssize_t bytesRead = 0;


        //  while loop to read from file
        while ((bytesRead = read(fd, buffer, sizeof(buffer))) > 0) {
         fprintf(file,"%s",buffer);

         // write to socket
         ssize_t bytesWritten = write(sockFd, buffer, bytesRead);
         // error handling
         if (bytesWritten <= 0) {
            perror("write to socket");
            close(fd);
            close(sockFd);
            return;
         }
      }
   fclose(file);
   close(fd);
   if (bytesRead < 0) {
      perror("read file");
   }
   printf("done sending content for: %s\n", path);
}
   close(sockFd);
}

void sendErrorResponse(int sockFd, int code, const char* message) {


      // for error html ouptut 
      char filename[100];
      pid_t pid = getpid();
      sprintf(filename,"tests/*error*client:%d.html",pid);
      FILE *file = fopen(filename,"w");
      if(file == NULL){
          perror("fopen");
          exit(EXIT_FAILURE);
      }

     // arragnge error output
    char header[1024] = "";
    strcat(header, "Content-Type: text/html\r\n");
    char content[1024] = "";
    sprintf(content, "<html><head><title>Error %d</title></head><body><h1>%d %s</h1></body></html>", code, code, message);
    //fprintf(file,"%d %s\n",code,message);
    // print error conent to ouptut file but not header (header only to client terminal)
    fprintf(file, "%s", content);
    fclose(file);
    char contentLength[50];
    sprintf(contentLength, "Content-Length: %ld\r\n\r\n", strlen(content));
    strcat(header, contentLength);

    write(sockFd, header, strlen(header));
    write(sockFd, content, strlen(content));
    close(sockFd);
}

