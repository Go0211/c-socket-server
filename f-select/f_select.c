#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <errno.h>

#define PORT 8080
#define MAX_CLIENTS 10

int main() {
    int server_fd, client_fd, max_fd, activity, valread;
    int client_sockets[MAX_CLIENTS] = {0};
    struct sockaddr_in address;
    socklen_t addrlen = sizeof(address);
    char buffer[1024];

    // 1. 소켓 생성
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == 0) {
        perror("socket 실패");
        exit(EXIT_FAILURE);
    }

    // 2. 바인딩
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    bind(server_fd, (struct sockaddr *)&address, sizeof(address));

    // 3. 리슨
    listen(server_fd, 5);
    printf("✅ 멀티클라이언트 서버 LISTEN 중...\n");

    fd_set readfds;

    while (1) {
        // fd set 초기화
        FD_ZERO(&readfds);
        FD_SET(server_fd, &readfds);
        max_fd = server_fd;

        // 클라이언트 소켓들을 fd set에 추가
        for (int i = 0; i < MAX_CLIENTS; i++) {
            int sd = client_sockets[i];
            if (sd > 0) FD_SET(sd, &readfds);
            if (sd > max_fd) max_fd = sd;
        }

        // select로 I/O 감지
        activity = select(max_fd + 1, &readfds, NULL, NULL, NULL);
        if ((activity < 0) && (errno != EINTR)) {
            perror("select 오류");
        }

        // 새 연결 요청 처리
        if (FD_ISSET(server_fd, &readfds)) {
            client_fd = accept(server_fd, (struct sockaddr *)&address, &addrlen);
            if (client_fd < 0) {
                perror("accept 실패");
                exit(EXIT_FAILURE);
            }
            printf("✅ 새 클라이언트 연결: fd=%d\n", client_fd);

            // 클라이언트 소켓 배열에 저장
            for (int i = 0; i < MAX_CLIENTS; i++) {
                if (client_sockets[i] == 0) {
                    client_sockets[i] = client_fd;
                    break;
                }
            }
        }

        // 기존 클라이언트에서 데이터 수신
        for (int i = 0; i < MAX_CLIENTS; i++) {
            int sd = client_sockets[i];
            if (FD_ISSET(sd, &readfds)) {
                valread = read(sd, buffer, sizeof(buffer));
                if (valread == 0) {
                    // 클라이언트 종료
                    printf("❌ 클라이언트 종료: fd=%d\n", sd);
                    close(sd);
                    client_sockets[i] = 0;
                } else {
                    buffer[valread] = '\0';
                    printf("📥 클라이언트(fd=%d): %s\n", sd, buffer);
                    send(sd, buffer, strlen(buffer), 0); // Echo
                }
            }
        }
    }

    return 0;
}

