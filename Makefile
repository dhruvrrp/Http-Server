
all : httpserver.c
	gcc -g -pthread -o httpS httpserver.c

clean :
	rm -f httpd *.o
