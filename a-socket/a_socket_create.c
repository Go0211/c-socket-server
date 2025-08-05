#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>

int main() {
    int sockfd;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        perror("소켓 생성 실패");
        exit(EXIT_FAILURE);
    }

    printf("✅ 소켓 생성 성공! FD: %d\n", sockfd);

    close(sockfd);
    return 0;
}

