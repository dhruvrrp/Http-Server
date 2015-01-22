#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>

char * docRoot;
int bufLen = 1024;
void handle_400(int clntSock)
{
	char *err_msg = "HTTP/1.0 400 Malformed Request";
	ssize_t byteSend = send(clntSock, err_msg, sizeof(err_msg), 0);
}
void th_handler(void * c)
{
	int clntSock = (int)c;
	printf("Someone connecteed!!! \n", c, clntSock);
//	printf("ip is %d \n", clntAddr.sin_addr.s_addr);
	char buffer[bufLen];
	ssize_t byteRecv = recv(clntSock, buffer, 1024, 0);
	
	printf("Data recieved : \n");
	fwrite(buffer, byteRecv, 1, stdout);
	char s[byteRecv];
	strcpy(s, buffer);
	
//	printf("%s \n", s);

	char * tok;
	tok = strtok(s, " \r\n");
	int len = 0;
	while(tok != NULL)
	{
	//	printf("%s \n", tok[0]);
		printf("%s \n", tok);
		len++;
		tok = strtok(NULL," \r\n");
	}
	char request[len][bufLen];
	tok = strtok(s, " \r\n");
	len = 0;
	while(tok != NULL)
	{
		strcpy(request[len], tok);
		len++;
		tok = strtok(NULL, " \r\n");
	}
	
	if(strcmp("GET", request[0]) != 0)
	{
		handle_400(clntSock);
	}
	else
	{
		char * docPath = request[1];
//		if(docPath + (strlen(docPath) - 1) == '/')
//			printf("Uhhhhhhhhh %d %s \n", strlen(docPath), );
//		FILE *fp = fopen(strcat(docPath, "MakeFile"), "r");

	}
	ssize_t byteSend = send(clntSock, buffer, byteRecv, 0);
	
	close(clntSock);
}


int main(int argc, char *argv[])
{

	if(argc != 3)
	{
		perror("Usage: <Port> <Document Root>\n");
		exit(1);
	}
	if(atoi(argv[1]) < 0 || atoi(argv[1]) > 65535)
	{
		printf("port number %s \n", argv[1] );
		perror("Incorrect port number\n");
		exit(1);
	}
	in_port_t servPort = atoi(argv[1]);
	docRoot = argv[2];
	printf(" hoho %c \n", docRoot[3]);
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

