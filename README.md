# SocketProgramming

1:1 채팅 프로그램 제작

## 구현 기능
- 서버와 클라이언트가 서로 Read / Write 된다.
- 클라이언트가 종료되면, 서버는 꺼지지 않는다.
- 클라이언트가 다시 접속하면, 채팅이 이뤄진다.
- 서버를 끄면, 클라이언트도 꺼진다.

## client.c
```c
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>

int fd;
char readbuf[100];
char writebuf[100];

int init() 
{
    int ret = 0;

    //1. socket
    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1) return -1;

    //2. connect
    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(12345);
    addr.sin_addr.s_addr = inet_addr("15.164.48.55");

    ret = connect(fd, (struct sockaddr *)&addr, sizeof(addr));
    if (ret == -1) return ret;

    return ret;
}

void bye()
{
    printf("\nBYE\n");
	
    shutdown(fd, SHUT_WR);

    close(fd);
    exit(1);
}

void* wr(){
	while(1){
		gets(writebuf);
		write(fd, writebuf, sizeof(writebuf));
		sleep(1);
	}
	return 0;
}

void* rd(){
	while(1){
		memset(readbuf, 0, 100);
		int rdcnt = read(fd, readbuf, sizeof(readbuf));
		if(rdcnt == 0) bye();
		printf("%s\n", readbuf);
		sleep(1);
	}
	return 0;
}

int main()
{
    signal(SIGINT, bye);

    int ret = init();
    if (ret == -1) printf("INIT ERROR\n");
    pthread_t r, w;
    pthread_create(&r, NULL, rd, NULL);
    pthread_create(&w, NULL, wr, NULL);

    pthread_join(r, NULL);
    pthread_join(w, NULL);

    return 0;
}
```

## server.c
```c
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
```
