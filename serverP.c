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
#define MYPORT "23047"	// the port used for UDP connection with Central Server
#define MAXBUFLEN 4000
#define CentralUDPPORT "24047" // the UDP port of Central Server
#define INF 999

int pre[1000];
char udp_buf[MAXBUFLEN] = "\0";

void shortPath(int s, int v, char *p[]){
    if(s == v){
       // printf("%d %s", s, p[s]);
        strcat(udp_buf, p[s]);
        strcat(udp_buf, ",");
        return;
    }
    shortPath(s, pre[v], p);
   // printf("%d %s", v, p[v]);
    strcat(udp_buf, p[v]);
    strcat(udp_buf, ",");
}


// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
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

// split received message
void split_args3(char *args[], char *message) {
    char *p = strtok (message, "\n");
    int i = 0;
    while (p != NULL)
    {
        args[i++] = p;
        p = strtok (NULL, "\n");
    }
}

// split received message
void split_args4(char *args[], char *message) {
    char *p = strtok (message, "/");
    int i = 0;
    while (p != NULL)
    {
        args[i++] = p;
        p = strtok (NULL, "/");
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
    
    printf("The Server P is up and running using UDP on port %s\n", MYPORT);
	
	while(1){

        if((numbytes = recvfrom(sockfd, buf, MAXBUFLEN-1, 0, 
			(struct sockaddr *)&their_addr, &addr_len)) == -1) {
            perror("recvfrom");
            exit(1);
        }
        buf[numbytes] = '\0';
		printf("The ServerP received the topology and score information\n");	
		//printf("%s", buf);
	    fflush(stdout);


        char *part[4];
        char *client[2];
        split_args1(part, buf);
        int clientA, clientB;

        //client hostname
        split_args2(client, part[0]);
        clientA = atoi(client[0]);
        clientB = atoi(client[1]);
        //printf("clientA= %d, clientB = %d\n", clientA, clientB);

        //hostname and its corresponding number
        char *host_num[1000];
        split_args4(host_num, part[2]);

        //scores
        char *score_line[1000];
        char *temp[2];
        int num_line;
        int num_host;
        float host[1000];

        //reconstruct graph
        char *graph_line[1000];
        int num_vertice;
        split_args3(graph_line, part[1]);
        num_vertice = strlen(graph_line[0]);
        float map[1000][1000];
        int i,j;
        int point;
        

        split_args3(score_line, part[3]);
        for(num_line = 0; num_line < num_vertice; num_line++){
            split_args2(temp, score_line[num_line]);
            for(num_host = 0; num_host < num_vertice; num_host++){
                if(strstr(score_line[num_line], host_num[num_host]) != NULL){
                    host[num_host] = atoi(temp[1]);
                }
            }
        }
        /*
         for(int test=0; test < num_vertice; test++){
            printf("host[%d] = %f\n", test,  host[test]);
        }
        */
        for(i = 0; i < num_vertice; i++){
            for(j = 0; j < num_vertice; j++){
                point = graph_line[i][j] - '0';
                if(point){
                    map[i][j] = (abs(host[i] - host[j]))/(host[i] + host[j]);
                }
                else{
                    map[i][j] = INF;    
                }
               // printf("%.2f ", map[i][j]);
            } 
          //  printf("\n");   
        }
        
        //Dijstrak. The implementation is based on code from CSDN with modifications
        int x, y;
        int vis[num_vertice];
        float dis[num_vertice];
       // int pre[num_vertice];
        int start = clientA;
        for(int z = 0; z < num_vertice; z++) pre[z] = z;
        memset(vis, 0, sizeof(vis));
        // memset(dis, 999.0, sizeof(float));
        for(int test = 0; test<num_vertice; test++){
            dis[test]=INF;
        }
        dis[start] = 0;
        for(x = 0; x < num_vertice; x++){
            int u = -1; 
            float min = INF;
            for(y = 0; y < num_vertice; y++){
                if(vis[y] == 0 && dis[y] < min){
				    u = y;
				    min = dis[y];
                }
			}
            if(u == -1) break;
            vis[u] = 1;
            for(int v = 0; v < num_vertice; v++){
			    if(vis[v] == 0 && map[u][v] != INF && dis[u] + map[u][v] < dis[v]){
				    dis[v] = dis[u] + map[u][v];
				    pre[v] = u;	
                }
            }
        }    
/*
        for(int test = 0; test<num_vertice; test++){
            printf("dis[%d]= %f\n", test, dis[test]);
        }
        for(int test = 0; test<num_vertice; test++){
            printf("pre[%d]= %d\n", test, pre[test]);
        }
*/
        //save results to buffer
       // char udp_buf[MAXBUFLEN] = "\0";
        char temp_dis[1000];
        memset(udp_buf, 0 , sizeof(udp_buf));

        if(dis[clientB] == INF){
            strcpy(udp_buf, "error;");
           // printf("%s\n", udp_buf);
        }
        else{
           // printf("Matching gap:%.2f\n", dis[clientB]);
            shortPath(clientA, clientB, host_num);
            strcat(udp_buf, ";");
            sprintf(temp_dis, "%.2f", dis[clientB]);
            strcat(udp_buf, temp_dis);
        }

      //  printf("%s", udp_buf);

		numbytes = sendto(sockfd, udp_buf, strlen(udp_buf), 0,
            (struct sockaddr *)&their_addr, addr_len);
        
        if(numbytes == -1) {
            perror("listener: sendto");
            exit(1);
        }		
		printf("The ServerP finished sending the results to the Central.\n");
        fflush(stdout);
    }
	
	return 0;
	
}