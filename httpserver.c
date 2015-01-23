#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
char * docRoot;

int bufLen = 1024;
void handle_400(int clntSock)
{

	char *err_msg = "HTTP/1.0 400 Bad Request \r\n \n";
//char *err_msg = "tessttt \r\n";	
printf(" sizee %d \n", sizeof(err_msg));
	ssize_t byteSend = send(clntSock, err_msg, strlen(err_msg), 0);
printf("byte send %d \n", byteSend);
}
void th_handler(void * c)
{
	struct stat staa;
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
	char ** request;
	while(tok != NULL)
	{
	//	printf("%s \n", tok[0]);
		request[len] = tok;
		printf("%s \n", request[len]);
		len++;
		tok = strtok(NULL," \r\n");
	}
	printf("Length: %d \n  ", len);
	char * docPath = request[1];
	printf("Docpath: %s \n", docPath);
	char * finalPath = (char *) malloc(1 + strlen(docPath) + strlen(docRoot));
	if(strcmp("GET", request[0]) != 0)
	{
		handle_400(clntSock);
	}
	else if(docPath[0] != '/')
	{
		handle_400(clntSock);
	}
	else
	{
		strcpy(finalPath, docRoot);
		strcat(finalPath, docPath);
		printf("Final Path: %s \n", finalPath);
		char * response = "HTTP/1.0 200 OK \r\n \n";
		send(clntSock, response, strlen(response), 0);
		int rr = open("Makefile", S_IROTH);
		fstat(rr, &staa);
		printf("size sent %d \n", staa.st_size);
		int qqq = sendfile(clntSock, rr, NULL, staa.st_size *2);
		printf("send file int %d \n", qqq);

//char * send_file
	}
	//	ssize_t byteSend = send(clntSock, buffer, byteRecv, 0);
//handle_400(clntSock);	
close(clntSock);
printf("Whattttt \n");
}


int main(int argc, char *argv[])
{
	docRoot = malloc(sizeof(char)*256);
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
	if(docRoot[strlen(docRoot) -1] == '/')
		docRoot[strlen(docRoot) -1] = '\0';

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
printf("Here core dumped \n");
	}
//	close(clntSock);
}

