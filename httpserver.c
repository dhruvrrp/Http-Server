#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/limits.h>
#include <time.h>
#include <errno.h>
#include <poll.h>
char * docRoot;

int bufLen = 1024;
/**void alarm_f(int s)
{
	signal(SIGALRM, SIG_IGN);
	printf("alaarrmm \n");
	signal(SIGALRM, alarm_f);
}**/
void handle_400(int clntSock)
{

	char *err_msg = "HTTP/1.1 400 Bad Request \r\n\r\n";	
	printf(" Error 400 %d \n", sizeof(err_msg));
	ssize_t byteSend = send(clntSock, err_msg, strlen(err_msg), 0);

}
void handle_404(int clntSock)
{

	char *err_msg = "HTTP/1.1 404 Not Found \r\n\r\n";	
	printf(" Error 404 %d \n", sizeof(err_msg));
	ssize_t byteSend = send(clntSock, err_msg, strlen(err_msg), 0);

}
void handle_403(int clntSock)
{

	char *err_msg = "HTTP/1.1 403 Forbidden \r\n\r\n";	
	printf(" Error 403 %d \n", sizeof(err_msg));
	ssize_t byteSend = send(clntSock, err_msg, strlen(err_msg), 0);

}

void *th_handler(void * c)
{
	struct stat staa;

	int clntSock = (int)c;
	printf("Someone connecteed!!! \n", c, clntSock);
//	printf("ip is %d \n", clntAddr.sin_addr.s_addr);
	char buffer[bufLen];
	int byteRecv = 0;
	int cuP = 0;
	do
	{
		byteRecv = recv(clntSock, buffer, bufLen, 0);
		cuP = cuP + byteRecv;
//	fwrite(buffer, byteRecv, 1, stdout);

		
	}
	while(buffer[cuP-1] != '\n' && buffer[cuP -2] != '\r' && buffer[cuP -3] != '\n' && buffer[cuP - 4] != '\r');

	
//	printf("Data recieved : \n");
	fwrite(buffer, byteRecv, 1, stdout);


	char s[byteRecv];

	strcpy(s, buffer);
	

//	printf("%s \n", s);

	char * tok;
	tok = strtok(s, " \r\n");
	int len = 0;
	char ** request;
	request = (char **) malloc(bufLen);
	while(tok != NULL)
	{
	//	printf("%s \n", tok[0]);
		request[len] = tok;
//		printf("%s \n", request[len]);
		len++;
		tok = strtok(NULL," \r\n");
//		printf("haha \n");
	}
	printf("Length: %d \n  ", len);
	char * docPath = request[1];
	printf("Docpath: %s \n", docPath);
//	char * finalPath;
	char * finalPath = (char *) malloc(1 + strlen(docPath) + strlen(docRoot) + 11);
	memset(finalPath, 0, sizeof(char) * strlen(finalPath));
	strcat(finalPath, docRoot);
	strcat(finalPath, docPath);
	char * absI = (char *) malloc(PATH_MAX + 1);
	memset(absI, 0, sizeof(char)*strlen(absI));
//	free(absI);
	char * hoho = realpath(finalPath, absI);
	printf("Final Path : %s \n Real path : %s \n", finalPath, absI);
	if(strcmp("GET", request[0]) != 0)
	{
		printf("Error \n");
		handle_400(clntSock);
	}
	else if(len < 3)
	{
		handle_400(clntSock);
	}
	else if(strstr(request[2], "HTTP") == NULL)
	{
		printf("Http error \n");
		handle_400(clntSock);
	}
	else if(docPath[0] != '/')
	{
		printf("400 \n");
		handle_400(clntSock);
	}
	else
	{
		char * response = (char *) malloc(bufLen);
		strcat(response, "HTTP/1.1 200 OK \r\n");
		int rr;
		printf("huh\n");
		struct stat sbuf;
		stat(absI, &sbuf);
		if(S_ISDIR(sbuf.st_mode))
		{
			if(absI[strlen(absI) - 1] != '/')
				strcat(absI, "/index.html");
			else
			{
				if(strlen(absI) == 1)
					absI = absI + 1;
				strcat(absI, "index.html");
			}
		//	if(absI[0] == '/')
		//		absI = absI + 1;
			printf("Path to file %s \n", absI);
			rr = open(absI, 0);
			strcat(response, "Content-Type: text/html \r\n");
			printf("File open %d\n", rr);
			if(rr == -1)
			{
				printf("Errno %s \n", strerror(errno));
				handle_404(clntSock);
			}

		}
		else
		{
	//		if(absI[0] == '/')
//				absI = absI + 1;
			rr = open(absI, 0);
			if(rr != -1)
			{
				printf("Is file %d\n", rr);
				char *token;
				token = strtok(absI, ".");
				printf("First strtok \n");
				char * ftype;
				if(token != NULL)
					token = strtok(NULL, ".");
				printf("Second strtok \n");
				if(strcmp(token, "png") == 0)
					ftype = "image/png";
				else if(strcmp(token, "jpeg") == 0)
					ftype = "image/jpeg";
				else if(strcmp(token, "html") == 0)
					ftype = "text/html";
				else
				{
					printf("Send 400 \n");
					rr = -1;
					handle_400(clntSock);
				}
				strcat(response, "Content-Type: ");
				strcat(response, ftype);
				strcat(response, "\r\n");
			}
			else
			{
				handle_404(clntSock);
			}
		}
		if(rr != -1)
		{
			printf("hoho \n");
			fstat(rr, &staa);
			if(staa.st_mode & S_IROTH)
			{
				int total = staa.st_size;
				printf("size sent %d \n", staa.st_size);
				strcat(response, "Content-Length: ");
				char * numS= (char *) malloc(200);;
				sprintf(numS, "%d", staa.st_size);
				strcat(response, numS);	
				strcat(response, "\r\n\r\n");

				int a = send(clntSock, response, strlen(response), 0);	

				int qqq = sendfile(clntSock, rr, NULL, staa.st_size);
			}
			else
			{
				handle_403(clntSock);
			}
		free(response);
		}
	}
	struct timeval tv;
	fd_set rfds;
	free(finalPath);
//	free(absI);
//	free(hoho);
	FD_ZERO(&rfds);
	FD_SET(clntSock, &rfds);
//	if(FD_ISSET(clntSock, &rfds))
//		printf("YESSS \n");
	tv.tv_sec = 10;
	tv.tv_usec = 0;
	int a = 0;

	if(len > 2 && strcmp(request[2], "HTTP/1.1") == 0)
	{
		printf("http 1.1 \n");
struct pollfd fds;
fds.fd = clntSock;
fds.events = POLLIN;
fds.revents = POLLIN;
a = poll(&fds, 1, 5000);
//		a = select(clntSock, &rfds, NULL, NULL, &tv);
		//{
			printf("Again !! %d\n", a);
	//		th_handler((void *) clntSock);
		//}/
	}
//	signal(SIGALRM, alarm_f);
//	alarm(5);

//	{
		printf("WHYYYS DSDSDSKDSKD %d\n", a);
//	}	
	if(a > 0)
		//th_handler((void *) clntSock);
	printf("End call \n");
	close(clntSock);

	pthread_exit(NULL);
}


int main(int argc, char *argv[])
{
	docRoot = malloc(sizeof(char)*256);
	if(argc != 3)
	{
		perror("Usage: <Port> <Document Root>\n");
		exit(1);
	}
	if(atoi(argv[1]) <= 0 || atoi(argv[1]) >= 65535)
	{
		printf("port number %s \n", argv[1] );
		perror("Incorrect port number\n");
		exit(0);
	}

	in_port_t servPort = atoi(argv[1]);
	docRoot = argv[2];
//	if(docRoot[strlen(docRoot) -1] == '/')
//		docRoot[strlen(docRoot) -1] = '\0';

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
	struct timeval tv;
	tv.tv_sec = 10;
	tv.tv_usec = 0;
	for(;;)
	{
printf("Start infinite \n");
		struct sockaddr_in clntAddr;
		socklen_t clntAddrLen = sizeof(clntAddr);

		int clntSock = accept(servSock, (struct sockaddr *) & clntAddr, &clntAddrLen);
//		setsockopt(clntSock, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv, sizeof(struct timeval));
	pthread_t cre;
		pthread_create(&cre, NULL, (void *) th_handler, (void *)clntSock);


	}
//	close(clntSock);
}

