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
    addr.sin_addr.s_addr = inet_addr("xx.xxx.xx.xx");

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
