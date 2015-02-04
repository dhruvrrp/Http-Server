
all : httpserver.c
	gcc -g -pthread -o httpd httpserver.c

clean :
	rm -f httpd *.o
