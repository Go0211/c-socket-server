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

// 비블로킹 소켓으로 설정하는 함수
int set_nonblocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) return -1;
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
    if (server_fd == -1) {
        perror("socket 실패");
        exit(EXIT_FAILURE);
    }

    // 2. 바인딩
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("bind 실패");
        exit(EXIT_FAILURE);
    }

    // 3. 리슨
    if (listen(server_fd, 10) < 0) {
        perror("listen 실패");
        exit(EXIT_FAILURE);
    }

    // 4. 서버 소켓 비블로킹 설정
    set_nonblocking(server_fd);

    // 5. epoll 인스턴스 생성
    epfd = epoll_create1(0);
    if (epfd == -1) {
        perror("epoll_create 실패");
        exit(EXIT_FAILURE);
    }

    // 6. 서버 소켓 epoll 등록
    ev.events = EPOLLIN;
    ev.data.fd = server_fd;
    epoll_ctl(epfd, EPOLL_CTL_ADD, server_fd, &ev);

    printf("✅ O_NONBLOCK 기반 서버 실행 중...\n");

    while (1) {
        int n = epoll_wait(epfd, events, MAX_EVENTS, -1);
        for (int i = 0; i < n; i++) {
            int fd = events[i].data.fd;

            if (fd == server_fd) {
                // 새 연결 수락
                while (1) {
                    client_fd = accept(server_fd, (struct sockaddr*)&address, &addrlen);
                    if (client_fd == -1) {
                        if (errno == EAGAIN || errno == EWOULDBLOCK) break;
                        perror("accept 실패");
                        break;
                    }

                    set_nonblocking(client_fd);

                    ev.events = EPOLLIN | EPOLLET;
                    ev.data.fd = client_fd;
                    epoll_ctl(epfd, EPOLL_CTL_ADD, client_fd, &ev);

                    printf("📥 새 연결: fd=%d\n", client_fd);
                }

            } else {
                // 데이터 수신
                while (1) {
                    ssize_t bytes = read(fd, buffer, sizeof(buffer) - 1);
                    if (bytes == -1) {
                        if (errno == EAGAIN || errno == EWOULDBLOCK) break;
                        perror("read 실패");
                        close(fd);
                        epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);
                        break;
                    } else if (bytes == 0) {
                        printf("❌ 클라이언트 종료: fd=%d\n", fd);
                        close(fd);
                        epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);
                        break;
                    } else {
                        buffer[bytes] = '\0';
                        printf("📨 fd=%d: %s\n", fd, buffer);
                        send(fd, buffer, bytes, 0);  // echo
                    }
                }
            }
        }
    }

    close(server_fd);
    close(epfd);
    return 0;
}

