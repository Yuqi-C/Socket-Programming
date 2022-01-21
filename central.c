#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>

#define PORTCLIENTA "25047"     // the port for TCP with client A
#define PORTCLIENTB "26047"     // the port for TCP with client B


#define UDPPORT "24047" // port number using UDP

#define SERVERPORTT "21047"	   // the port users will be connecting to 
							   // Backend-Server (A)
#define SERVERPORTS "22047"	   // the port users will be connecting to 
							   // Backend-Server (B)
#define SERVERPORTP "23047"	   // the port users will be connecting to 
							   // Backend-Server (C)
#define IPADDRESS "127.0.0.1"  // local IP address

#define BACKLOG 10	 // how many pending connections queue will hold

#define MAPNOTFOUND "Map not found"

#define MAXBUFLEN 4000

// function of reaping all dead processes
void sigchld_handler(int s){
    // waitpid() might overwrite errno, so we save and restore it:
    int saved_errno = errno;
    
    while(waitpid(-1, NULL, WNOHANG) > 0);
    
    errno = saved_errno;
}

// funciton to get socket address from a sockaddr struct, from beej
void *get_in_addr(struct sockaddr *sa) {
    if(sa->sa_family == AF_INET6) {
        // IPv6
        return &(((struct sockaddr_in6*)sa)->sin6_addr);
    }
    // IPv4
    return &(((struct sockaddr_in*)sa)->sin_addr);
}

// split received message
void split_args1(char *args[], char *message) {
    char *p = strtok (message, ";");
    int i = 0;
    while (p != NULL)
    {
        args[i++] = p;
        p = strtok (NULL, ";");
    }
}

// split received message
void split_args2(char *args[], char *message) {
    char *p = strtok (message, " ");
    int i = 0;
    while (p != NULL)
    {
        args[i++] = p;
        p = strtok (NULL, " ");
    }
}

// parse the calculated results
void parse_result(char *args[], char *message) {
    char *p = strtok (message, "\n");
    int i = 0;
    while (p != NULL)
    {
        args[i++] = p;
        p = strtok (NULL, "\n");
    }
}

// setup TCP with client at port
int setupTCP(char* port) {
    int rv;
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    struct sigaction sa;
    int yes = 1;
	
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if((rv = getaddrinfo(IPADDRESS, port, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
    }

    for(p = servinfo; p != NULL; p = p->ai_next) {
        if((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            perror("server: socket");
            continue;
        }
        if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1){
            perror("setsockopt");
            exit(1);
        }
               if(bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("server: bind");
            continue;
        }
        break;
    }

    freeaddrinfo(servinfo);

    if(p == NULL) {
        fprintf(stderr, "server: failed to bind\n");
        exit(1);
    }

    if(listen(sockfd, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }
    
    sa.sa_handler = sigchld_handler; // reap all dead processes sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1); 
    }

    return sockfd;
}

// Just create UDP socket, no send and receive operations
int setupUDP(char* port)
{
	int sockfd;
	int rv;
	struct addrinfo hints, *servinfo, *p;
	socklen_t addr_len;
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;

	if ((rv = getaddrinfo(IPADDRESS, port, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and make a socket
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("talker: socket");
			continue;
		}

        if(bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("server: bind");
            continue;
        }

		break;
	}

	if (p == NULL) {
		fprintf(stderr, "talker: failed to create socket\n");
		return 2;
	}

	freeaddrinfo(servinfo); // done with servinfo
	return sockfd;
}


// use UDP socket on central server to query each backend server
void udpQuery(int sockfd, char *query, char *port, char *data) {
    //int server_sock;
	int recv_bytes;
    int numbytes;
    int rv;
    struct addrinfo hints, *servinfo, *p;
	socklen_t addr_len;
	memset(&hints, 0, sizeof hints);
    char recv_data[MAXBUFLEN]; // data received from backend server
       
    if ((rv = getaddrinfo(IPADDRESS, port, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return;
	}

    for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("talker: socket");
			continue;
		}
        break;
    }

    if (p == NULL) {
		fprintf(stderr, "talker: failed to create socket\n");
		return;
	}
    
    // send map_id, source_vtx and target_vtx to server
	if ((numbytes = sendto(sockfd, query, strlen(query), 0, p->ai_addr, p->ai_addrlen)) == -1) {
		perror("talker: sendto");
		exit(1);
	}

	if (strcmp(port, SERVERPORTT)==0) {
		printf("The central server sent a request to Backend-Server T\n");
	} else if (strcmp(port, SERVERPORTS)==0) {
		printf("The central server sent a request to Backend-Server S\n");
    } else {
        printf("The central server sent a processing request to Backend-Server P\n");
    }

    recv_bytes = recvfrom(sockfd, recv_data, sizeof recv_data, 0, NULL, NULL);
    if(recv_bytes == -1) {
        perror("recvfrom");
        exit(1);
    }
    recv_data[recv_bytes] = '\0';
    strcpy(data, recv_data);
		
	if (strcmp(port, SERVERPORTT)==0) {
		printf("The Central server received information from Backend-Server T using UDP over port 21047\n");
	} else if (strcmp(port, SERVERPORTS)==0) {
		printf("The Central server received information from Backend-Server S using UDP over port 22047\n");
	} else{
		printf("The Central server received the results from backend server P\n");
	}
	
}

int main(){
	int sockfd_A, sockfd_B,new_fd_A, new_fd_B;
    int sockfd_UDP;
	struct sockaddr_storage clientA_addr;
	struct sockaddr_storage clientB_addr;
	socklen_t sin_size; // used for accept()
	char s[INET6_ADDRSTRLEN];
	int numbytes; //using in receive or send
	char message_buf_A[100];
	char message_buf_B[100];
	char udp_send[1000];
    char udp_C_to_S[MAXBUFLEN];
    char udp_C_to_P[MAXBUFLEN];
	char udp_Treceived[MAXBUFLEN]; 
	char udp_Sreceived[MAXBUFLEN]; 
	char udp_Preceived[MAXBUFLEN];

    socklen_t sin_size_A;
    socklen_t sin_size_B;

	sockfd_A = setupTCP(PORTCLIENTA);
	sockfd_B = setupTCP(PORTCLIENTB);
	sockfd_UDP = setupUDP(UDPPORT);
	
	
	printf("The Central server is up and running\n");
	
	
	while(1){

		sin_size_A = sizeof clientA_addr;
		sin_size_B = sizeof clientB_addr;
		
		new_fd_A = accept(sockfd_A, (struct sockaddr *)&clientA_addr, &sin_size_A);
		
		if(new_fd_A == -1) {
            perror("A accept");
            continue;
        }
		
		inet_ntop(clientA_addr.ss_family, get_in_addr((struct sockaddr *)&clientA_addr), s, sizeof s);
        //printf("server: got connection from %s\n", s);
		
		if(!fork()) {
         //   close(sockfd_A);

            // receive arguments as single string
            if((numbytes = recv(new_fd_A, message_buf_A, sizeof message_buf_A, 0)) == -1) {
                perror("recv");
                exit(1);
            }
            message_buf_A[numbytes] = '\0';
            // printf("message from clientA = %s\n",message_buf_A);
            printf("The Central server received input = < %s > from the client A using TCP over port<25047>\n", message_buf_A);
		
            new_fd_B = accept(sockfd_B, (struct sockaddr *)&clientB_addr, &sin_size_B);
		
	    	if(new_fd_B == -1) {
              perror("B accept");
                continue;
            }
		
		    inet_ntop(clientB_addr.ss_family, get_in_addr((struct sockaddr *)&clientB_addr), s, sizeof s);
            //printf("server: got connection from %s\n", s);

		    if(!fork()) {
             //   close(sockfd_B);

                // receive arguments as single string
                if((numbytes = recv(new_fd_B, message_buf_B, sizeof message_buf_B, 0)) == -1) {
                     perror("recv");
                     exit(1);
                }
    			message_buf_B[numbytes] = '\0';
                // printf("message from clientB = %s\n",message_buf_B);
	            printf("The Central server received input = < %s > from the client B using TCP over port<26047>\n", message_buf_B);
		        fflush(stdout);
                char *temp_error;
                memset(udp_Treceived, 0, sizeof(udp_Treceived));
                memset(udp_Sreceived, 0, sizeof(udp_Sreceived));
                memset(udp_Preceived, 0, sizeof(udp_Preceived));

                sprintf(udp_send, "%s %s", message_buf_A, message_buf_B);

		        udpQuery(sockfd_UDP, udp_send, SERVERPORTT, udp_Treceived);
	    	    udpQuery(sockfd_UDP, udp_Treceived, SERVERPORTS, udp_Sreceived);
		        udpQuery(sockfd_UDP, udp_Sreceived, SERVERPORTP, udp_Preceived);	    
               
                //   printf("%s\n", udp_Preceived);
                
                if(strstr(udp_Preceived, "error") != NULL){
                    strcat(udp_Preceived, udp_send);
                }

                //    printf("%s\n", udp_Preceived);
            

                if (send(new_fd_B, udp_Preceived, strlen(udp_Preceived), 0) == -1) 
				    perror("send");

                printf("The Central server sent the results to client<B>\n");

                if (send(new_fd_A, udp_Preceived, strlen(udp_Preceived), 0) == -1) 
                    perror("send");

                printf("The Central server sent the results to client<A>\n");

		        close(new_fd_B);
                exit(0);
		        
            }
			close(new_fd_A);
            exit(0);
        }

		close(new_fd_A);
		close(new_fd_B);
		
	}	
	
	return 0;
}			