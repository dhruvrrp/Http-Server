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
/**************************************
 * 
 * NAME: handle_400
 * 
 * DESCRIPTION: send 400 bad request response
 * 
 * INPUT: 
 *      PARAMETERS:
 * 			int			clientSocket		Socket at which client connects
 * 		GLOBAL:
 * 			None
 * 		RETURN:
 * 			None
 * 
 */
void handle_400(int clientSocket)
{

	char *err_msg = "HTTP/1.1 400 Bad Request \r\n\r\n";	
	printf(" Error 400 %ld \n", sizeof(err_msg));
	ssize_t byteSend = send(clientSocket, err_msg, strlen(err_msg), 0);

}
/**************************************
 * 
 * NAME: handle_404
 * 
 * DESCRIPTION: send 404 Not Found response
 * 
 * INPUT: 
 *      PARAMETERS:
 * 			int			clientSocket		Socket at which client connects
 * 		GLOBAL:
 * 			None
 * 		RETURN:
 * 			None
 * 
 */
void handle_404(int clientSocket)
{

	char *err_msg = "HTTP/1.1 404 Not Found \r\n\r\n";	
	printf(" Error 404 %ld \n", sizeof(err_msg));
	ssize_t byteSend = send(clientSocket, err_msg, strlen(err_msg), 0);

}
/**************************************
 * 
 * NAME: handle_403
 * 
 * DESCRIPTION: send 403 Forbidden response
 * 
 * INPUT: 
 *      PARAMETERS:
 * 			int			clientSocket		Socket at which client connects
 * 		GLOBAL:
 * 			None
 * 		RETURN:
 * 			None
 * 
 */
void handle_403(int clientSocket)
{

	char *err_msg = "HTTP/1.1 403 Forbidden \r\n\r\n";	
	printf(" Error 403 %ld \n", sizeof(err_msg));
	ssize_t byteSend = send(clientSocket, err_msg, strlen(err_msg), 0);

}
/**************************************
 * 
 * NAME: void th_handler
 * 
 * DESCRIPTION: Handle the http request of the client connected
 * 
 * INPUT: 
 *      PARAMETERS:
 * 			void * 		c			Pointer to the client socket which
 * 									connected to our server
 * 		GLOBAL:
 * 			None
 * 		RETURN:
 * 			None
 * 
 */
void *th_handler(void * c)
{
	struct stat staa;
	// Cast client socket to int to utilize
	int clientSocket = (int)c;
	printf("Someone connecteed!!! \n", c, clientSocket);
	// Create msgBuffer for request and totalRecv to track size of request
	char msgBuffer[bufLen];
	int totalRecv = 0;

	// loop to keep recieving data till two CRLF signifing end of request
	do
	{
		totalRecv += recv(clientSocket, msgBuffer, bufLen, 0);
	}
	while(msgBuffer[totalRecv-1] != '\n' 
			&& msgBuffer[totalRecv -2] != '\r' 
			&& msgBuffer[totalRecv -3] != '\n' 
			&& msgBuffer[totalRecv - 4] != '\r');


	// Copy recieved message to a char * to be able to tokenize it
	char s[totalRecv];

	strcpy(s, msgBuffer);
	

	// Tokenize the request using CRLF which seperates parts of request

	char * tokens;
	tokens = strtok(s, " \r\n");
	int count = 0;
	char ** request;
	request = (char **) malloc(bufLen);
	// Iterate over all tokens while caculating number of tokens
	while(tokens != NULL)
	{
		request[count] = tokens;
		count++;
		tokens = strtok(NULL," \r\n");
	}

	// Extract path to the file requested and generate the real path to use
	// with our file system

	char * docPath = request[1];
	char * finalPath = (char *) malloc(1 + strlen(docPath) + strlen(docRoot) + 11);
	memset(finalPath, 0, sizeof(char) * strlen(finalPath));
	strcat(finalPath, docRoot);
	strcat(finalPath, docPath);

	char * realPath = (char *) malloc(PATH_MAX + 1);
	memset(realPath, 0, sizeof(char)*strlen(realPath));
	char * resolved = realpath(finalPath, realPath);

	// Error if request type is not GET
	if(strcmp("GET", request[0]) != 0)
	{
		printf("Error \n");
		handle_400(clientSocket);
	}
	// If tokens <3 then we do not have enough information to process request
	else if(count < 3)
	{
		handle_400(clientSocket);
	}
	// Cannot handle protocols other than HTTP
	else if(strstr(request[2], "HTTP") == NULL)
	{
		printf("Http error \n");
		handle_400(clientSocket);
	}
	// Document path is malformed causing error
	else if(docPath[0] != '/')
	{
		printf("400 \n");
		handle_400(clientSocket);
	}
	else
	{
		// Start forming response for cliient
		char * response = (char *) malloc(bufLen);
		strcat(response, "HTTP/1.1 200 OK \r\n");
		int fileDescriptor;
		struct stat fileStat;
		stat(realPath, &fileStat);
		// if file pointed by path is a folder, return the index.html for that 
		// folder
		if(S_ISDIR(fileStat.st_mode))
		{
			if(realPath[strlen(realPath) - 1] != '/')
				strcat(realPath, "/index.html");
			else
			{
				if(strlen(realPath) == 1)
					realPath = realPath + 1;
				strcat(realPath, "index.html");
			}

			fileDescriptor = open(realPath, 0);
			strcat(response, "Content-Type: text/html \r\n");

			// If index.html does not exist for the folder, send 404 error
			if(fileDescriptor == -1)
			{
				printf("Errno %s \n", strerror(errno));
				handle_404(clientSocket);
			}

		}
		else
		{
			// If path does not lead to a folder, open file
			fileDescriptor = open(realPath, 0);

			if(fileDescriptor != -1)
			{
				printf("Is file %d\n", fileDescriptor);
				// Tokenize path to find out file type and add it to response
				// {filename.filetype}
				char *token;
				token = strtok(realPath, ".");
				// Ignore file name token
				char * ftype;
				if(token != NULL)
					token = strtok(NULL, ".");
				if(strcmp(token, "png") == 0)
					ftype = "image/png";
				else if(strcmp(token, "jpeg") == 0)
					ftype = "image/jpeg";
				else if(strcmp(token, "html") == 0)
					ftype = "text/html";
				else
				{
					// Unidentified file type, send 400 error
					printf("Send 400 \n");
					fileDescriptor = -1;
					handle_400(clientSocket);
				}
				strcat(response, "Content-Type: ");
				strcat(response, ftype);
				strcat(response, "\r\n");
			}
			else
			{
				handle_404(clientSocket);
			}
		}
		if(fileDescriptor != -1)
		{
			printf("hoho \n");
			fstat(fileDescriptor, &staa);
			if(staa.st_mode & S_IROTH)
			{
				int total = staa.st_size;
				printf("size sent %ld \n", staa.st_size);
				strcat(response, "Content-Length: ");
				char * numS= (char *) malloc(200);;
				sprintf(numS, "%ld", staa.st_size);
				strcat(response, numS);	
				strcat(response, "\r\n\r\n");

				int a = send(clientSocket, response, strlen(response), 0);	

				int qqq = sendfile(clientSocket, fileDescriptor, NULL, staa.st_size);
			}
			else
			{
				handle_403(clientSocket);
			}
		free(response);
		}
	}
	struct timeval tv;
	fd_set rfds;
	free(finalPath);
//	free(realPath);
//	free(hoho);
	FD_ZERO(&rfds);
	FD_SET(clientSocket, &rfds);
//	if(FD_ISSET(clientSocket, &rfds))
//		printf("YESSS \n");
	tv.tv_sec = 10;
	tv.tv_usec = 0;
	int a = 0;

	if(count > 2 && strcmp(request[2], "HTTP/1.1") == 0)
	{
		printf("http 1.1 \n");
struct pollfd fds;
fds.fd = clientSocket;
fds.events = POLLIN;
fds.revents = POLLIN;
a = poll(&fds, 1, 5000);
//		a = select(clientSocket, &rfds, NULL, NULL, &tv);
		//{
			printf("Again !! %d\n", a);
	//		th_handler((void *) clientSocket);
		//}/
	}
//	signal(SIGALRM, alarm_f);
//	alarm(5);

//	{
		printf("WHYYYS DSDSDSKDSKD %d\n", a);
//	}	
	if(a > 0)
		//th_handler((void *) clientSocket);
	printf("End call \n");
	close(clientSocket);

	pthread_exit(NULL);
}

/**************************************
 * 
 * NAME: void main
 * 
 * DESCRIPTION: Control function for http server, listens on specified port
 * 				and spawns threads to handle client connections
 * 
 * INPUT: 
 *      PARAMETERS:
 * 			int			argc		Command-line arguments to create start 
 * 									server. Takes in the port to listen on
 * 									and the path to root of the folder to 
 * 									server requests
 * 		GLOBAL:
 * 			None
 * 		RETURN:
 * 			None
 * 
 */
void main(int argc, char *argv[])
{
	//Allocate space for the document root

	docRoot = malloc(sizeof(char)*256);

	//Check if port number is valid

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


	//Create the server socket using defaults
	
	int servSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(servSock < 0)
	{
		perror("Incorrect parameter\n");
		exit(1);
	}

	//Construct local socket address structure
	
	struct sockaddr_in servAddr;
	memset(&servAddr, 0, sizeof(servAddr));
	servAddr.sin_family = AF_INET;
	servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	
	servAddr.sin_port = htons(servPort);

	//Bind socket Address to servSock and listen for connections
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

	// Infinite loop which blocks at accept and spawns a new pthread
	// whenever a new client connect appears
	for(;;)
	{
		struct sockaddr_in clntAddr;
		socklen_t clntAddrLen = sizeof(clntAddr);
		// On a new connection, accept returns the client socket which we utilize
		// to communicate with the client
		int clientSocket = accept(servSock, (struct sockaddr *) & clntAddr, &clntAddrLen);
		pthread_t cre;
		// Spawn thread and exceute th_handler to serve the request
		pthread_create(&cre, NULL, (void *) th_handler, (void *)clientSocket);


	}
}

