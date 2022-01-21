
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

#include <ctype.h>

#define PORT "25047"    // the TCP port of central server which client connects to
#define IPADDRESS "127.0.0.1" // source and destination address
#define MAXBUFLEN 4000   // the maximum number of bytes

// funciton to get socket address from a sockaddr struct
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
    char *p = strtok (message, ",");
    int i = 0;
    while (p != NULL)
    {
        args[i++] = p;
        p = strtok (NULL, ",");
    }
}
// split received message
void split_args3(char *args[], char *message) {
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

// format printing
void print_result(char *args[]) {
    printf("Found compatibility for <%s> and <%s>:\n", args[0], args[1]);
    //printf("<%s> -- <%s> -- <%s> -- <%s>\n", args[0], args[1], args[0], args[1]);
    //printf("-------------------------------------------------------------------------------\n");
    printf("Compatibility score:%s\n", args[2]);
}

int main(int argc, char *argv[]) {

    int sockfd, numbytes;  
    char buf[MAXBUFLEN];
    char *results[5];
    //char args_str[100];
    char check[10];
    struct addrinfo hints, *servinfo, *p;
    int rv;
    char s[INET6_ADDRSTRLEN];

    // check whether aguments enough
    if(argc != 2) {
        fprintf(stderr, "usage Error: client function needs 1 argument <hostname> ");
        exit(1);
    }

	//intialization
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
	
    // load up address structs with getaddrinfo()
    if ((rv = getaddrinfo(IPADDRESS, PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and connect to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("client: socket");
            continue;
        }

        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            perror("client: connect");
            close(sockfd);
            continue;
        }

        break;
    }

    if (p == NULL) {
        fprintf(stderr, "client: failed to connect\n");
        return 2;
    }
	
	/*
    inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr), s, sizeof s);
	*/
	
    printf("The client is up and running.\n");

    freeaddrinfo(servinfo); // all done with this structure

    if ((numbytes = send(sockfd, argv[1], strlen(argv[1]), 0)) == -1) {
        perror("send");
        exit(1);
    }
    fflush(stdout);

    printf("The client sent <%s> to the Central Server\n", argv[1]);

    if((numbytes = recv(sockfd, buf, MAXBUFLEN-1, 0)) == -1) {
        perror("recv");
        exit(1);
    }

    buf[numbytes] = '\0';
   //printf("%s\n",buf);


    char *rcv_part[2];
    char *error_part[2];
    char *error_piece[2];
    char *host_num[1000];
    int check0, check1, check2; 
    char hosts[MAXBUFLEN];
    int count_num = 0;


    if(strstr(buf,"error") != NULL){
        split_args1(error_part, buf);
        split_args3(error_piece ,error_part[1]);
        printf("Found no compatibility for <%s> and <%s>\n", error_piece[0],error_piece[1]);
    }
    else{
        split_args1(rcv_part, buf);
        strcpy(hosts, rcv_part[0]);
       // printf("==%s", hosts);

        for(check0 = 0; hosts[check0]!='\0'; check0++){
            if(hosts[check0] == ',') {
                count_num++;
            }    
        }
       // printf("count_num = %d", count_num);
        split_args2(host_num, rcv_part[0]);
        printf("Found compatibility for <%s> and <%s>:\n", host_num[0], host_num[count_num - 1]);
        for(check1 = 0; check1 < count_num - 1; check1++){
            printf("<%s>--", host_num[check1]);
        }
        printf("<%s>\n", host_num[count_num - 1]);
        printf("Matching Gap:<%s>\n", rcv_part[1]);
    
    }

    close(sockfd);

    return 0;
}