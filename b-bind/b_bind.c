#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define PORT 8080 // 사용할 포트번호

int main() {
    int server_fd;
    struct sockaddr_in address;

    // 1. 소켓 생성
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("소켓 생성 실패");
        exit(EXIT_FAILURE);
    }
    printf("✅ 소켓 생성 성공! FD: %d\n", server_fd);

    // 2. 주소 구조체 초기화
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;              // IPv4
    address.sin_addr.s_addr = INADDR_ANY;      // 모든 네트워크 인터페이스 허용
    address.sin_port = htons(PORT);            // 포트 번호 (네트워크 바이트 오더)

    // 3. 소켓에 IP/Port 바인딩
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind 실패");
        close(server_fd);
        exit(EXIT_FAILURE);
    }
    printf("✅ 소켓에 IP/Port 바인딩 성공!\n");

    // 4. 소켓 종료
    close(server_fd);
    return 0;
}

