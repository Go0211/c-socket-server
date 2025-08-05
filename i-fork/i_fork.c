#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>
#include <sys/wait.h>

#define PORT 8080
#define BUFFER_SIZE 1024

void handle_client(int client_fd) {
    char buffer[BUFFER_SIZE];
    int n;

    while ((n = read(client_fd, buffer, sizeof(buffer) - 1)) > 0) {
        buffer[n] = '\0';
        printf("📥 클라이언트(fd=%d): %s\n", client_fd, buffer);
        send(client_fd, buffer, strlen(buffer), 0); // Echo
    }

    printf("❌ 클라이언트 종료: fd=%d\n", client_fd);
    close(client_fd);
    exit(0);
}

// 좀비 프로세스 방지용 시그널 핸들러
void sigchld_handler(int sig) {
    (void)sig;
    while (waitpid(-1, NULL, WNOHANG) > 0);
}

int main() {
    int server_fd, client_fd;
    struct sockaddr_in address;
    socklen_t addrlen = sizeof(address);

    signal(SIGCHLD, sigchld_handler); // 자식 종료 시 좀비 방지

    // 1. 소켓 생성
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("socket 실패");
        exit(EXIT_FAILURE);
    }

    // 2. 주소 설정 및 바인딩
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    bind(server_fd, (struct sockaddr*)&address, sizeof(address));

    // 3. 리슨
    listen(server_fd, 5);
    printf("✅ fork 기반 서버 실행 중...\n");

    // 4. 클라이언트 대기 루프
    while (1) {
        client_fd = accept(server_fd, (struct sockaddr*)&address, &addrlen);
        if (client_fd < 0) {
            perror("accept 실패");
            continue;
        }

        pid_t pid = fork();
        if (pid < 0) {
            perror("fork 실패");
            close(client_fd);
        } else if (pid == 0) {
            // 자식 프로세스
            close(server_fd);  // 자식은 서버 소켓 닫음
            handle_client(client_fd);
        } else {
            // 부모 프로세스
            close(client_fd);  // 부모는 클라이언트 소켓 닫음
        }
    }

    return 0;
}

