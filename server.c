#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>

int fd;
int new_fd;
int flag = 0;
char writebuf[100];
char readbuf[100];

int init1()
{
    int ret = 0;
    fd = socket(AF_INET, SOCK_STREAM, 0);

    //1. socket
    if (fd == -1) {
	    printf("socket error\n");
	    return -1;
    }

    //2. binding
    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(12345);
    addr.sin_addr.s_addr = 0;
    ret = bind(fd, (struct sockaddr *)&addr, sizeof(struct sockaddr));
    if (ret == -1) {
	    printf("bind error\n");
	    printf("Error code: %d\n", errno);
	    return ret;
    }
}

int init2(){
    //3. listen state 
    int ret = 0;
    ret = listen(fd, 2);
    if (ret == -1) {
	    printf("listen error\n");
	    return ret;
    }
    printf("LISTEN...\n");
    return ret;
}

void bye()
{
    printf("\nbye\n");
    shutdown(new_fd, SHUT_WR);

    close(new_fd);
    close(fd);
    exit(0);
}

void* rd(){
   while(1){
	memset(readbuf, 0, 100);
	int rdcnt = read(new_fd, readbuf, sizeof(readbuf));
	if(rdcnt == 0) {
	    printf("client exit\n");
	    flag = 1;
	    return;;
	}
	printf("%s\n", readbuf);
	sleep(1);
   }
}

void* wr(){
   while(1){
	gets(writebuf);
	if(flag == 1) return;
	write(new_fd, writebuf, sizeof(writebuf));
   }
}

int main()
{

    flag = 0;
    signal(SIGINT, bye);

    pthread_t r, w;


    int ret = init1();
    if (ret == -1){
	printf("INIT1 ERROR\n");
    }
    
    while(1){
    ret = init2();
    if(ret == -1){
    	printf("INIT2 ERROR\n");
	break;
    }

    //accept (blocking)
    struct sockaddr new_addr = {0};
    int len;
    new_fd = accept(fd, &new_addr, &len);
    printf("START\n");

    //loop
    pthread_create(&r, NULL, rd, NULL);
    pthread_create(&w, NULL, wr, NULL);
    
    pthread_join(r, NULL);
    pthread_join(w, NULL);

    printf("byebye\n");
    close(new_fd);
    }
    
    close(fd);
    
    return 0;
}
