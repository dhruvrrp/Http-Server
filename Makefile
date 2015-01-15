all : httpserver.c
	gcc -pthread -o httpd httpserver.c

clean :
	rm httpd
