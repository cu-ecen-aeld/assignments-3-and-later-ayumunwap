#include <sys/types.h>
#include <sys/socket.h>
#include <syslog.h>	// syslog
#include <stdio.h>	// fopen
#include <stdlib.h>	// exit
#include <netdb.h>	// addrinfo
#include <arpa/inet.h>	// inet_ntop
#include <unistd.h>	// close
#include <signal.h>	// sigaction
#include <string.h>
#include <errno.h>	// perror prints a message followed by the textual representation of errno

#define BACKLOG 10     // how many pending connections queue will hold
#define BUFFER_SIZE 32768

int sockfd = -1;
FILE *file = NULL;
struct addrinfo *resaddr = NULL;

//  i. completing any open connection operations, closing any open sockets, and deleting the file
void close_sock() {
    if (sockfd != -1) {
        shutdown(sockfd, SHUT_RDWR);
        close(sockfd);
    }
    if (file != NULL) fclose(file);
    remove("/var/tmp/aesdsocketdata");
    if (resaddr != NULL) freeaddrinfo(resaddr);
}

void sighandler(int signal) {
    // Logs message to the syslog when SIGINT or SIGTERM is received
    syslog(LOG_ERR, "Caught signal %d, exiting", signal);
    close_sock();
    exit(0);
}

int main(int argc, char *argv[])
{
    file = fopen("/var/tmp/aesdsocketdata", "w+"); // creating this file if it doesn’t exist
    if (file == NULL) {
        perror("Error opening /var/tmp/aesdsocketdata");
        close_sock();
        return -1;
    }

    struct sigaction sa;
    sa.sa_handler = sighandler;
    sa.sa_flags = 0; 
    sigemptyset(&sa.sa_mask);
    if (sigaction(SIGINT, &sa, NULL) != 0 || sigaction(SIGTERM, &sa, NULL) != 0) {
        exit(0);
    }

// b. Opens a stream socket bound to port 9000
    struct addrinfo hints;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_flags = AI_PASSIVE;
    hints.ai_family = AF_INET;	// IPv4
    hints.ai_socktype = SOCK_STREAM;	// TCP
    if (getaddrinfo(NULL, "9000", &hints, &resaddr) != 0) {
        perror("getaddrinfo");
        close_sock();
        return -1;
    }

    sockfd = socket(resaddr->ai_family, resaddr->ai_socktype, resaddr->ai_protocol);
    if (sockfd == -1) {
        perror("socket");
        close_sock();
        return -1;
    }
    
    if (bind(sockfd, resaddr->ai_addr, resaddr->ai_addrlen) != 0) {
        perror("bind");
        close_sock();
        return -1;
    }

    // When in daemon mode the program should fork after ensuring it can bind to port 9000
    if (argc == 2 && strcmp(argv[1], "-d") == 0) {
    	printf("in daemon mode\n");
        // exit parent
        if (fork() > 0) exit(0);
    } else {
        printf("in native mode\n");
    }
       
// c. Listens for a connection
    if (listen(sockfd,  BACKLOG) != 0) {
        perror("listen");
        close_sock();
        return -1;
    }

//  h. Restarts accepting connections from new clients forever in a loop until SIGINT or SIGTERM is received
    while (1) {
// c. accepts a connection
        struct sockaddr *saddr = resaddr->ai_addr;
        socklen_t saddrlen = sizeof(saddr);
        int acceptedfd = accept(sockfd, saddr, &saddrlen);
        if (acceptedfd == -1) {
	    perror("accept");
            close_sock();
            return -1;
	}
	    
// d. Logs message to the syslog with the IP address of the connected client
	char ip4[INET_ADDRSTRLEN];  // space to hold the IPv4 string
	struct sockaddr_in *saddr_in = (struct sockaddr_in *)resaddr->ai_addr;
	inet_ntop(AF_INET, &(saddr_in->sin_addr), ip4, INET_ADDRSTRLEN);
	syslog(LOG_INFO, "Accepted connection from %s", ip4);

// e. Receives data over the connection and appends to file
        char buffer[BUFFER_SIZE];
        if (recv(acceptedfd, buffer, sizeof(buffer), 0) == -1) {
	    perror("recv");
            close_sock();
            return -1;
	}    
	// Use a newline to separate data packets received
	size_t bytes_received = strchr(buffer, '\n') - buffer + 1;
	fwrite(buffer, sizeof(char), bytes_received, file);

// f. Returns the full content of the file to the client as soon as the received data packet completes
	fseek(file, 0, SEEK_END);
	long file_size = ftell(file);
	fseek(file, 0, SEEK_SET);
	fread(buffer, sizeof(char), file_size, file);
	if (send(acceptedfd, buffer, file_size, 0) == -1) {
	    perror("send");
            close_sock();
            return -1;
	}

//  g. Logs message to the syslog with the IP address of the connected client
	syslog(LOG_INFO, "Closed connection from %s", ip4);
    }
 }
