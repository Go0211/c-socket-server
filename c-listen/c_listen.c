#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define PORT 8080

int main() {
    int server_fd;
    struct sockaddr_in address;

    // 1. 소켓 생성
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("소켓 생성 실패");
        exit(EXIT_FAILURE);
    }

    // 2. 주소 초기화 및 바인딩
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind 실패");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // 3. listen() 호출
    if (listen(server_fd, 5) < 0) {
        perror("listen 실패");
        close(server_fd);
        exit(EXIT_FAILURE);
    }
    printf("✅ 서버가 LISTEN 상태에 들어감. 클라이언트 연결 대기 중...\n");

    // 서버 종료
    close(server_fd);
    return 0;
}

