#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main() {
    int sockfd;
    char buffer[BUFFER_SIZE];
    struct sockaddr_in servaddr, cliaddr;
    socklen_t len;

    // 1. 소켓 생성
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("socket 실패");
        exit(EXIT_FAILURE);
    }

    // 2. 서버 주소 설정 및 바인딩
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(PORT);

    if (bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("bind 실패");
        exit(EXIT_FAILURE);
    }

    printf("✅ UDP 서버 실행 중...\n");

    while (1) {
        len = sizeof(cliaddr);
        int n = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&cliaddr, &len);
        buffer[n] = '\0';

        printf("📨 클라이언트: %s\n", buffer);

        sendto(sockfd, buffer, n, 0, (struct sockaddr *)&cliaddr, len);  // Echo
    }

    close(sockfd);
    return 0;
}

