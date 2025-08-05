#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define PORT 8080

int main() {
    int server_fd, client_fd;
    struct sockaddr_in address;
    socklen_t addrlen = sizeof(address);

    // 1. 소켓 생성
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("소켓 생성 실패");
        exit(EXIT_FAILURE);
    }

    // 2. 바인딩
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind 실패");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // 3. 리슨
    if (listen(server_fd, 5) < 0) {
        perror("listen 실패");
        close(server_fd);
        exit(EXIT_FAILURE);
    }
    printf("✅ 서버가 LISTEN 상태에 들어감\n");

    // 4. 클라이언트 연결 수락
    client_fd = accept(server_fd, (struct sockaddr *)&address, &addrlen);
    if (client_fd < 0) {
        perror("accept 실패");
        close(server_fd);
        exit(EXIT_FAILURE);
    }
    printf("✅ 클라이언트 연결 수락! client_fd: %d\n", client_fd);

    // 5. 클라이언트와 데이터 송수신
    char buffer[1024] = {0};
    read(client_fd, buffer, sizeof(buffer));
    printf("📥 클라이언트로부터 받은 메시지: %s\n", buffer);

    write(client_fd, "Hello from server", 17);
    printf("📤 클라이언트에게 메시지 전송 완료\n");

    // 6. 종료
    close(client_fd);
    close(server_fd);

    return 0;

    // nc 127.0.0.1 8080 -> 이걸로 테스트 입력하는 과정 가능!
}

