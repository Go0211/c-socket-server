#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define PORT 8080
#define MAX_EVENTS 10
#define BUFFER_SIZE 1024

int set_nonblocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

int main() {
    int server_fd, client_fd, epfd;
    struct sockaddr_in address;
    socklen_t addrlen = sizeof(address);
    struct epoll_event ev, events[MAX_EVENTS];
    char buffer[BUFFER_SIZE];

    // 1. 소켓 생성
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    bind(server_fd, (struct sockaddr*)&address, sizeof(address));
    listen(server_fd, 5);
    set_nonblocking(server_fd);

    // 2. epoll 인스턴스 생성
    epfd = epoll_create1(0);

    // 3. 서버 소켓을 epoll에 등록
    ev.events = EPOLLIN;
    ev.data.fd = server_fd;
    epoll_ctl(epfd, EPOLL_CTL_ADD, server_fd, &ev);

    printf("✅ epoll 서버 실행 중...\n");

    while (1) {
        int n = epoll_wait(epfd, events, MAX_EVENTS, -1);
        for (int i = 0; i < n; i++) {
            int fd = events[i].data.fd;

            if (fd == server_fd) {
                // 새 클라이언트 연결
                client_fd = accept(server_fd, (struct sockaddr*)&address, &addrlen);
                set_nonblocking(client_fd);

                ev.events = EPOLLIN | EPOLLET;  // 엣지 트리거(고성능)
                ev.data.fd = client_fd;
                epoll_ctl(epfd, EPOLL_CTL_ADD, client_fd, &ev);
                printf("📥 새 연결: fd=%d\n", client_fd);
            } else {
                // 데이터 수신
                int bytes = read(fd, buffer, BUFFER_SIZE);
                if (bytes <= 0) {
                    close(fd);
                    epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);
                    printf("❌ 클라이언트 종료: fd=%d\n", fd);
                } else {
                    buffer[bytes] = '\0';
                    printf("📨 fd=%d: %s\n", fd, buffer);
                    send(fd, buffer, bytes, 0);  // Echo
                }
            }
        }
    }

    close(server_fd);
    close(epfd);
    return 0;
}

