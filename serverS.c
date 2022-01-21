#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <ctype.h>

#define IPADDRESS "127.0.0.1"  // local IP address
#define MYPORT "22047"	// the port used for UDP connection with Central Server
#define SCOREDIR "./scores.txt" // file directory to get compatibility test scores
#define MAXBUFLEN 4000
#define CentralUDPPORT "24047" // the UDP port of Central Server


// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

// split received message
void split_args(char *args[], char *message) {
    char *p = strtok (message, " ");
    int i = 0;
    while (p != NULL)
    {
        args[i++] = p;
        p = strtok (NULL, " ");
    }
}

int main(void) {
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int rv;
    int numbytes;
    struct sockaddr_storage their_addr;
    socklen_t addr_len;
    char s[INET6_ADDRSTRLEN];

    char buf[MAXBUFLEN];
    char data[MAXBUFLEN];
    char *args[3];

	 memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;

    // get information of backend server itself
    if((rv = getaddrinfo(IPADDRESS, MYPORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

	// loop through all the results and make a socket
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            perror("server: socket");
            continue;
        }

        if(bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("server: bind");
            continue;
        }
        break;
    }

	if(p == NULL) {
        fprintf(stderr, "talker: failed to bind socket\n");
        return 2;
    }

    freeaddrinfo(servinfo);

    addr_len = sizeof their_addr;
    
    printf("The ServerS is up and running using UDP on port %s.\n", MYPORT);
	
	while(1){
        if((numbytes = recvfrom(sockfd, buf, MAXBUFLEN-1, 0, 
			(struct sockaddr *)&their_addr, &addr_len)) == -1) {
            perror("recvfrom");
            exit(1);
        }
        buf[numbytes] = '\0';
		printf("The ServerS received a request from Central to get the scores.\n");	
		//printf("%s", buf);
        fflush(stdout);
    
        char F_buf[MAXBUFLEN];
		FILE *fp = fopen(SCOREDIR, "r");

        if (fp == NULL)
	    {
		    perror("fail to read");
		    exit(1);
    	}

         while(fgets(F_buf, sizeof(F_buf), fp) != NULL)
		{   
            F_buf[strlen(F_buf)] = '\0';
            strcat(buf, F_buf);
		}

        strcat(buf, ";");
      //  printf("%s", udp_score);

	    numbytes = sendto(sockfd, buf, strlen(buf), 0,
            (struct sockaddr *)&their_addr, addr_len);
        
        if(numbytes == -1) {
            perror("listener: sendto");
            exit(1);
        }		
		printf("The ServerS finished sending the scores to Central.\n");	
        fflush(stdout);
    }
	
	return 0;
	
}