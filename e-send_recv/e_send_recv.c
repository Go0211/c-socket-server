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
    char buffer[1024] = {0};

    // 1. 소켓 생성
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // 2. 바인딩
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    bind(server_fd, (struct sockaddr *)&address, sizeof(address));

    // 3. 리슨
    listen(server_fd, 5);
    printf("✅ 서버가 LISTEN 상태\n");

    // 4. 클라이언트 연결 수락
    client_fd = accept(server_fd, (struct sockaddr *)&address, &addrlen);
    if (client_fd < 0) {
        perror("accept");
        close(server_fd);
        exit(EXIT_FAILURE);
    }
    printf("✅ 클라이언트 연결 수락!\n");

    // 5. 데이터 수신 (recv)
    int bytes_read = recv(client_fd, buffer, sizeof(buffer), 0);
    if (bytes_read > 0) {
        printf("📥 클라이언트로부터 받은 데이터: %s\n", buffer);
    }

    // 6. 데이터 송신 (send)
    const char *msg = "Hello from server (E단계)";
    send(client_fd, msg, strlen(msg), 0);
    printf("📤 클라이언트에게 데이터 전송 완료\n");

    // 7. 종료
    close(client_fd);
    close(server_fd);

    return 0;
}

