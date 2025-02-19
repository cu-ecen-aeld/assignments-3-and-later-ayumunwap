#include <sys/types.h>
#include <sys/socket.h>
#include <syslog.h>	// syslog
#include <stdio.h>	// fopen
#include <netdb.h>	// addrinfo
#include <arpa/inet.h>	// inet_ntop
#include <unistd.h>	// close
#include <signal.h>	// sigaction
#include <stdlib.h>	// exit
#include <string.h>

#define BACKLOG 10     // how many pending connections queue will hold
#define BUFFER_SIZE 32768

int main(int argc, char *argv[])
{
    openlog("aesdsocket", LOG_PID | LOG_CONS, LOG_USER);
    
// b. Opens a stream socket bound to port 9000
    struct addrinfo hints, *resaddr;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_flags = AI_PASSIVE;
    hints.ai_family = AF_INET;		// IPv4
    hints.ai_socktype = SOCK_STREAM;	// TCP
    if (getaddrinfo(NULL, "9000", &hints, &resaddr) != 0) {
        syslog(LOG_ERR, "getaddrinfo() error");
	closelog();
        return -1;
    }
    int client_sockfd = socket(resaddr->ai_family, resaddr->ai_socktype, resaddr->ai_protocol);
    if (client_sockfd == -1) {
        syslog(LOG_ERR, "sock creation error");
	closelog();
        return -1;
    }
    if (bind(client_sockfd, resaddr->ai_addr, resaddr->ai_addrlen) != -0) {
        syslog(LOG_ERR, "bind() error");
	closelog();
        return -1;
    }

    // When in daemon mode the program should fork after ensuring it can bind to port 9000
    pid_t pid = fork();

    // parent
    if (pid > 0) exit(0);

    // child
    else if (pid == 0) {
        // open /var/tmp/aesdsocketdata, creating this file if it doesn’t exist
        FILE *file = fopen("/var/tmp/aesdsocketdata", "w+");
        if (file == NULL) {
            syslog(LOG_ERR, "Error opening /var/tmp/aesdsocketdata");
	    closelog();
            return -1;
        }
       
// c. Listens for a connection
        if (listen(client_sockfd,  BACKLOG) != -0) {
            syslog(LOG_ERR, "listen() error");
	    closelog();
            return -1;
        }

        while (1) {
//  h. Restarts accepting connections from new clients forever in a loop until SIGINT or SIGTERM is received
            if (sigaction(SIGINT, NULL, NULL) != 0 || sigaction(SIGTERM, NULL, NULL) != 0) {
                // Logs message to the syslog when SIGINT or SIGTERM is received
                syslog(LOG_ERR, "Caught signal, exiting");
                break;
            }

// c. accepts a connection
            struct sockaddr *saddr = resaddr->ai_addr;
            socklen_t saddrlen = sizeof(saddr);
	    int acceptedfd = accept(client_sockfd, saddr, &saddrlen);
	    if (acceptedfd == -1) {
	        syslog(LOG_ERR, "accept() error");
	        break;
	    }
	    
// d. Logs message to the syslog with the IP address of the connected client
	    char ip4[INET_ADDRSTRLEN];  // space to hold the IPv4 string
	    struct sockaddr_in *saddr_in = (struct sockaddr_in *)resaddr->ai_addr;
	    inet_ntop(AF_INET, &(saddr_in->sin_addr), ip4, INET_ADDRSTRLEN);
	    syslog(LOG_INFO, "Accepted connection from %s", ip4);

// e. Receives data over the connection and appends to file
            char buffer[BUFFER_SIZE];
       	    if (recv(acceptedfd, buffer, sizeof(buffer), 0) == -1) {
	        syslog(LOG_ERR, "recv() error");
	        break;
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
	        syslog(LOG_ERR, "send() error");
	        break;
	    }

//  g. Logs message to the syslog with the IP address of the connected client
	    syslog(LOG_INFO, "Closed connection from %s", ip4);
	}

//  i. completing any open connection operations, closing any open sockets, and deleting the file
        shutdown(client_sockfd, SHUT_RDWR);
        close(client_sockfd);
        fclose(file);
        remove("/var/tmp/aesdsocketdata");
        freeaddrinfo(resaddr);
        closelog();
        return -1;
    }
}
