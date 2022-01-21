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
#include <math.h>

#define IPADDRESS "127.0.0.1"  // local IP address
#define MYPORT "21047"	// the port used for UDP connection with Central Server
#define TOPODIR "./edgelist.txt" // file directory to get social network graph
//#define TOPODIR "/home/student/Desktop/edgelist.txt" // file directory to get social network graph
#define MAXBUFLEN 4000
#define CentralUDPPORT "24047" // the UDP port of Central Server

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

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
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
        fprintf(stderr, "talker: failed to bind socket.\n");
        return 2;
    }

    freeaddrinfo(servinfo);

    addr_len = sizeof their_addr;
    
    printf("The ServerT is up and running using UDP on port %s.\n", MYPORT);
	
	while(1){
       if((numbytes = recvfrom(sockfd, buf, MAXBUFLEN-1, 0, 
			(struct sockaddr *)&their_addr, &addr_len)) == -1) {
            perror("recvfrom");
            exit(1);
        }

        buf[numbytes] = '\0';
		printf("The ServerT received a request from Central to get the topology.\n");	
		//printf("hostnames are %s\n", buf);
	    fflush(stdout);
        

        split_args(args, buf);
	    //printf("args[0] = %s,args[1] = %s\n", args[0], args[1]);

	    //extract from the edgelist.txt by line 
		char *single_path[1000];
		FILE *fp = fopen(TOPODIR, "r");
		char F_buf[MAXBUFLEN];
		char i = 0;
        int line = 0;
	    
		char temp_host[1000][30]={"\0"};
        char info[1000][100]={"\0"};
		char *token[2];
        char host[1000][100];
        char data[MAXBUFLEN] = {"\0"};

	    if (fp == NULL)
	    {
		    perror("fail to read");
		    exit(1);
    	}

        while(fgets(F_buf, sizeof(F_buf), fp) != NULL)
		{   
            F_buf[strlen(F_buf) - 1] = '\0';
            strcpy(info[line], F_buf);
            //printf("info[%d] = %s\n", line, info[line]);
            fflush(stdout);
            line++;
            split_args(token, F_buf);
            strcpy(temp_host[i], token[0]);
            //printf("temp_host[%d] = %s\n", i, temp_host[i]);
            strcpy(temp_host[++i], token[1]);
            //printf("temp_host[%d] = %s\n", i, temp_host[i]);
			i++;
            fflush(stdout);
		}

		fclose(fp);	  

        
        int detect;
        char num_vertice = 0;
        char big = 0,small = 0;
        for(big = 0; big < i; big++){
            detect = 0;
            for(small = 0; small < big; small++){
                if( (strcmp(temp_host[big],temp_host[small])) == 0) {
                    detect = 1;
                    //printf("big = %d\n", big);
                }
            } 
            if(!detect){
                strcpy(host[num_vertice],temp_host[big]);
               // printf("host[%d] = %s\n", num_vertice, host[num_vertice]);
                fflush(stdout);
                num_vertice++;
            }    
        }
        //printf("num_vertice = %d, line = %d\n", num_vertice, line);

        //number corresponding to hostnames of clientA and B
        char arr;
        char x,y;
        char hostAB[1000];
        for(arr = 0; arr < num_vertice; arr++){
            if(strcmp(host[arr], args[0]) == 0){
                x = arr; 
            }        
        }

        for(arr = 0; arr < num_vertice; arr++){
            if(strcmp(host[arr], args[1]) == 0){
                y = arr; 
            }        
        }
    
        sprintf(hostAB,"%d %d;",x, y);
        strcpy(data, hostAB);
        //printf("data = %s", data);

        //Create graph
        int a, b;
        int c[2];
        int E_flag;
        int map[1000][1000];
        memset(map, 0, sizeof(map));
        for(a = 0; a < line; a++){
            E_flag = 0;
            for(b = 0; b < num_vertice; b++){
                if(strstr(info[a], host[b]) != NULL){
                    c[E_flag] = b;
                    E_flag++; 
                } 
            }       
           // printf("c[0] = %d, c[1] = %d\n", c[0],c[1]);           
            map[c[0]][c[1]]= map[c[1]][c[0]] = 1;
        }

        char test_a, test_b;
        for(test_a = 0; test_a < num_vertice; test_a++){
            for(test_b = 0; test_b < num_vertice; test_b++){
                if(map[test_a][test_b]){
                    strcat(data,"1");
                }
                else{
                    strcat(data,"0");
                }
            }
            strcat(data, "\n");
        } 
        strcat(data,";");

       // printf("data = %s", data);

        //hostname and corresponding number
        int number;
        char temp[1000];
        for(number = 0; number < num_vertice; number++){
           // printf("host[%d] = %s", number, host[number]);
            sprintf(temp, "%s/", host[number]);
            strcat(data, temp);
        }

        strcat(data,";");

        //printf("x = %d, y = %d\n", x, y);
       // printf("data = %s\n", data);
        fflush(stdout);

		numbytes = sendto(sockfd, data, strlen(data), 0,
                    (struct sockaddr *)&their_addr, addr_len);
        
        if(numbytes == -1) {
            perror("listener: sendto");
            exit(1);
        }		
		printf("The ServerT finished sending the topology to Central.\n");
		fflush(stdout);
    }
	
	return 0;
	
	
}