#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define PORT 8080
#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024

int client_sockets[MAX_CLIENTS] = {0};
pthread_mutex_t client_lock = PTHREAD_MUTEX_INITIALIZER;

void* handle_client(void* arg) {
    int client_fd = *(int*)arg;
    free(arg);

    char buffer[BUFFER_SIZE];
    int valread;

    while ((valread = read(client_fd, buffer, sizeof(buffer) - 1)) > 0) {
        buffer[valread] = '\0';
        printf("📥 클라이언트(fd=%d): %s\n", client_fd, buffer);
        send(client_fd, buffer, strlen(buffer), 0);  // echo
    }

    if (valread == 0) {
        printf("❌ 클라이언트 종료: fd=%d\n", client_fd);
    } else {
        perror("read 실패");
    }

    close(client_fd);

    // client_sockets[]에서 제거
    pthread_mutex_lock(&client_lock);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (client_sockets[i] == client_fd) {
            client_sockets[i] = 0;
            break;
        }
    }
    pthread_mutex_unlock(&client_lock);

    return NULL;
}

int main() {
    int server_fd, client_fd;
    struct sockaddr_in address;
    socklen_t addrlen = sizeof(address);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket 실패");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("bind 실패");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 5) < 0) {
        perror("listen 실패");
        exit(EXIT_FAILURE);
    }

    printf("✅ pthread 기반 멀티스레드 서버 실행 중...\n");

    while (1) {
        client_fd = accept(server_fd, (struct sockaddr*)&address, &addrlen);
        if (client_fd < 0) {
            perror("accept 실패");
            continue;
        }

        printf("✅ 새 클라이언트 연결: fd=%d\n", client_fd);

        pthread_mutex_lock(&client_lock);
        int assigned = 0;
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (client_sockets[i] == 0) {
                client_sockets[i] = client_fd;
                assigned = 1;
                break;
            }
        }
        pthread_mutex_unlock(&client_lock);

        if (!assigned) {
            printf("❌ 최대 클라이언트 수 초과: 연결 종료\n");
            close(client_fd);
            continue;
        }

        int* pclient = malloc(sizeof(int));
        *pclient = client_fd;

        pthread_t tid;
        if (pthread_create(&tid, NULL, handle_client, pclient) != 0) {
            perror("pthread_create 실패");
            close(client_fd);
            continue;
        }

        pthread_detach(tid);  // 스레드 자원 자동 해제
    }

    close(server_fd);
    return 0;
}

