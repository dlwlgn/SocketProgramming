# SocketProgramming

1:1 채팅 프로그램 제작

## 구현 기능
- 서버와 클라이언트가 서로 Read / Write 된다.
- 클라이언트가 종료되면, 서버는 꺼지지 않는다.
- 클라이언트가 다시 접속하면, 채팅이 이뤄진다.
- 서버를 끄면, 클라이언트도 꺼진다.

## client.c
```c
#include <arpa/inet.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

int fd;
char readbuf[100];
char writebuf[100];

int init()
{
    int ret = 0;

    //1. socket
    fd = socket(AF_INET, SOCK_STREAM, 0);  //IPv4, 양방향, TCP로 소켓 생성
    if (fd == -1) return -1;

    //2. connect
    struct sockaddr_in addr = {0};                     //구조체 선언
    addr.sin_family = AF_INET;                         //IPv4
    addr.sin_port = htons(12345);                      //빅엔디안으로 변환
    addr.sin_addr.s_addr = inet_addr("15.164.48.55");  //통신할 서버 ip 주소 입력

    ret = connect(fd, (struct sockaddr*)&addr, sizeof(addr));  //위에 선언된 구조체 정보로 서버에 연결
    if (ret == -1) return ret;

    return ret;
}

void bye()
{
    printf("\nBYE\n");

    shutdown(fd, SHUT_WR);  //서버로 길이 0을 보낸다.(EOF)

    close(fd);  //클라이언트 소켓 종료
    exit(1);
}

void* wr()
{
    while (1) {
        gets(writebuf);                         //문자열을 입력받아 writebuf에 저장
        write(fd, writebuf, sizeof(writebuf));  //writebuf 내부 문자열을 소켓으로 쓰기
    }
    return 0;
}

void* rd()
{
    while (1) {
        memset(readbuf, 0, 100);                         //종료를 막기 위해 버퍼에 길이 100 할당
        int rdcnt = read(fd, readbuf, sizeof(readbuf));  //클라이언트 소켓으로 읽어서 readbuf에 저장
        if (rdcnt == 0) bye();                           //서버로부터 길이 0을 받았을 때(EOF) bye 함수 실행
        printf("%s\n", readbuf);                         //readbuf에 저장된 내용 출력
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
#include <arpa/inet.h>
#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

int fd;
int new_fd;
int flag = 0;
char writebuf[100];
char readbuf[100];

//socket 함수 및 bind 함수 실행
int init1()
{
    //1. socket
    int ret = 0;
    fd = socket(AF_INET, SOCK_STREAM, 0);  //IPv4, 양방향, TCP로 socket 파일디스크립터 정의
    if (fd == -1) {
        printf("socket error\n");
        return -1;
    }

    //2. binding
    struct sockaddr_in addr = {0};                                     //구조체 선언
    addr.sin_family = AF_INET;                                         //양방향 통신 설정
    addr.sin_port = htons(12345);                                      //네트워크 바이트 정렬 방식(빅엔디안)으로 변환
    addr.sin_addr.s_addr = 0;                                          //localhost(서버 자기 자신) 지칭
    ret = bind(fd, (struct sockaddr*)&addr, sizeof(struct sockaddr));  //위의 구조체로 bind
    if (ret == -1) {
        printf("bind error\n");
        printf("Error code: %d\n", errno);
        return ret;
    }
}

int init2()
{
    //3. listen state
    int ret = 0;
    ret = listen(fd, 2);  //네트워크 시스템의 들어오는 연결 요청을 저장하는 큐(accept 전까지)
    //2는 큐의 크기, 대기열 중 가장 오래된 연결 요청과 연결한다.

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
    shutdown(new_fd, SHUT_WR);  //클라이언트로 길이 0을 보낸다. (EOF)

    close(new_fd);  //클라이언트 종료
    close(fd);      //서버 종료
    exit(0);        //프로그램 종료
}

void* rd()
{
    while (1) {
        memset(readbuf, 0, 100);                             //실행되자마자 꺼지는 것을 방지하기 위해 길이 100 할당
        int rdcnt = read(new_fd, readbuf, sizeof(readbuf));  //클라이언트로부터 읽어서 readbuf에 저장한다. (rdcnt엔 길이가 저장된다.)
        if (rdcnt == 0) {                                    //길이가 0이 왔을 때(EOF)
            printf("client exit\n");                         //클라이언트가 종료되었다는 메시지를 보낸다.
            flag = 1;                                        //wr 프로세스를 종료해주기 위한 flag
            return;
            ;
        }
        printf("%s\n", readbuf);  //read로 읽어낸 내용 출력
        sleep(1);
    }
}

void* wr()
{
    while (1) {
        gets(writebuf);                             //문자열 입력
        if (flag == 1) return;                      //클라이언트가 종료했을 경우 wr 프로세스 종료
        write(new_fd, writebuf, sizeof(writebuf));  //입력한 문자열 클라이언트로 쓰기
    }
}

int main()
{
    flag = 0;
    signal(SIGINT, bye);  //컨트롤 c 입력시 bye 함수 실행

    pthread_t r, w;  //멀티 스레드 실행을 위한 선언

    int ret = init1();  //init1 실행
    if (ret == -1) {
        printf("INIT1 ERROR\n");
    }

    while (1) {         //클라이언트가 나가도 listen 상태로 복귀
        ret = init2();  //init2 실행
        if (ret == -1) {
            printf("INIT2 ERROR\n");
            break;
        }

        //accept (blocking)
        struct sockaddr new_addr = {0};
        int len;
        new_fd = accept(fd, &new_addr, &len);  //클라이언트가 접속할 때까지 기다린다.(block)
        printf("START\n");                     //접속시 메시지 출력

        //loop
        pthread_create(&r, NULL, rd, NULL);  //rd 함수 멀티 스레스 실행
        pthread_create(&w, NULL, wr, NULL);  //wr 함수 멀티 스레드 실행

        pthread_join(r, NULL);  //rd 함수 스레드 종료
        pthread_join(w, NULL);  //wr 함수 스레드 종료

        printf("byebye\n");
        close(new_fd);  //클라이언트 종료
    }                   //다시 클라이언트 대기 상태로 복귀

    close(fd);

    return 0;
}
```
