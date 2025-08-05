#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 8080
#define BUFFER_SIZE 1024

int sock;  // 공유 FD
volatile int running = 1;  // 종료 신호

void* send_thread(void* arg) {
    char buffer[BUFFER_SIZE];

    while (running) {
        if (!fgets(buffer, sizeof(buffer), stdin)) break;

        buffer[strcspn(buffer, "\n")] = '\0';  // 개행 제거

        if (strcmp(buffer, "exit") == 0) {
            running = 0;
            break;
        }

        send(sock, buffer, strlen(buffer), 0);
    }

    return NULL;
}

void* recv_thread(void* arg) {
    char buffer[BUFFER_SIZE];
    int n;

    while (running) {
        n = recv(sock, buffer, sizeof(buffer) - 1, 0);
        if (n <= 0) {
            printf("❌ 서버 연결 종료됨\n");
            running = 0;
            break;
        }

        buffer[n] = '\0';
        printf("📨 서버: %s\n", buffer);
    }

    return NULL;
}

int main() {
    struct sockaddr_in server_addr;
    pthread_t tid_send, tid_recv;

    // 1. 소켓 생성
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("소켓 생성 실패");
        exit(EXIT_FAILURE);
    }

    // 2. 서버 주소 설정
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr);

    // 3. 서버 연결
    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("서버 연결 실패");
        close(sock);
        exit(EXIT_FAILURE);
    }

    printf("✅ 서버에 연결됨 (%s:%d)\n", SERVER_IP, SERVER_PORT);

    // 4. 입력/수신 스레드 생성
    pthread_create(&tid_send, NULL, send_thread, NULL);
    pthread_create(&tid_recv, NULL, recv_thread, NULL);

    // 5. 스레드 종료 대기
    pthread_join(tid_send, NULL);
    pthread_join(tid_recv, NULL);

    // 6. 정리
    close(sock);
    printf("👋 클라이언트 종료\n");
    return 0;
}

