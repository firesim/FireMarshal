#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <time.h> 
#include <fcntl.h>

int main(int argc, char *argv[])
{
    if(argc != 2)
    {
        printf("\nUsage: %s <num_clients>\n",argv[0]);
        return 1;
    } 

    int num_clients = *(int *)(argv[1]);
    
    int listenfd = 0, connfd = 0;
    struct sockaddr_in serv_addr; 

    char sendBuff[1025];
    time_t ticks; 

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    memset(&serv_addr, '0', sizeof(serv_addr));
    memset(sendBuff, '0', sizeof(sendBuff)); 

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(5000); 

    bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)); 

    listen(listenfd, 10);

    for (int i = 0; i < 2; i += 1) {
        
        connfd = accept(listenfd, (struct sockaddr*)NULL, NULL); 
	
	printf("Sending\n");
        snprintf(sendBuff, sizeof(sendBuff), "172.16.0.2\r\n");
        write(connfd, sendBuff, strlen(sendBuff));
       	printf("Closing\n");	
        close(connfd);
    } 

    close(listenfd);

    return 0;

}
