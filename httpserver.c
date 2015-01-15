#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>


void th_handler(void * c)
{
	int clntSock = (int)c;
	printf("Someone connecteed!!!", c, clntSock);
//	printf("ip is %d \n", clntAddr.sin_addr.s_addr);
	char buffer[1024];
	ssize_t byteRecv = recv(clntSock, buffer, 1024, 0);
	
	printf("Data recieved : \n");
	fwrite(buffer, byteRecv, 1, stdout);
	char s[byteRecv];
	strcpy(s, buffer);
	
//	printf("%s \n", s);

	char * tok;
	tok = strtok(s, " \r");
	while(tok != NULL)
	{
		printf("%s x\n", tok);
		tok = strtok(NULL," \r");
	}
	ssize_t byteSend = send(clntSock, buffer, byteRecv, 0);
	
	close(clntSock);
}


int main(int argc, char *argv[])
{
	if(argc != 2)
	{
		perror("Incorrect parameter\n");
		exit(1);
	}
	in_port_t servPort = atoi(argv[1]);
	
	//Create socket
	
	int servSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(servSock < 0)
	{
		perror("Incorrect parameter\n");
		exit(1);
	}

	//Construct local address structure
	
	struct sockaddr_in servAddr;
	memset(&servAddr, 0, sizeof(servAddr));
	servAddr.sin_family = AF_INET;
	servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	
	servAddr.sin_port = htons(servPort);

	if(bind(servSock, (struct sockaddr*) &servAddr, sizeof(servAddr)) < 0)
	{
		perror("bind() failed");
		exit(1);
	}
	
	if(listen(servSock, 5) < 0)
	{
		perror("listen() failed");
		exit(1);
	}

	for(;;)
	{
		struct sockaddr_in clntAddr;
		socklen_t clntAddrLen = sizeof(clntAddr);
	
		int clntSock = accept(servSock, (struct sockaddr *) & clntAddr, &clntAddrLen);
		pthread_t cre;
		pthread_create(&cre, NULL, (void *) &th_handler, (void *)clntSock);
	}
//	close(clntSock);
}

