#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <arpa/inet.h>
#include <time.h>


int main(int argc, char *argv[])
{
	int sockfd = 0, res = 0, n = 0;
	char recvBuff[1024];
	struct sockaddr_in serv_addr; 
	fd_set myset; 
	struct timeval tv; 
	int valopt; 
	socklen_t lon; 

	if(argc != 2)
	{
		printf("\nUsage: %s <ip of server>\n",argv[0]);
		return 1;
	} 

	memset(&serv_addr, '0', sizeof(serv_addr)); 

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(5000); 

	if(inet_pton(AF_INET, argv[1], &serv_addr.sin_addr)<=0)
	{
		printf("\nError : inet_pton error occured\n");
		return 1;
	} 

	memset(recvBuff, '0',sizeof(recvBuff));

	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		printf("\nError : Could not create socket (%d %s) \n", errno, strerror(errno));
		return 1;
	} 


	int t = 0;
	while(1){

		if(t >= 30)	
		{
			printf("\nError : Could not connect to server (%d %s) \n", errno, strerror(errno));
			return 1;
		}
		if(connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) >= 0) {
			break;
		}
		sleep(1);

		t += 1;	
	}


	while ((n = read(sockfd, recvBuff, sizeof(recvBuff)-1)) > 0)
	{
		recvBuff[n] = 0;
		if(fputs(recvBuff, stdout) == EOF)
		{
			printf("\nError : Fputs error\n");
		}
	} 

	if(n < 0)
	{
		printf("\nError : Read error\n");
	} else {
		if (strncmp(recvBuff,argv[1],strlen(argv[1])) == 0) {
			printf("\nSuccess : %s\n", argv[1]);
		} else {
			printf("\n Error : Expected : %s Received : %s\n", argv[1], recvBuff);
		}
	}

	close(sockfd);

	return 0;
}
