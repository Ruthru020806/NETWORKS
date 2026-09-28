#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define SERVER_IP "127.0.0.1"
#define PORT 7000
#define BUFFER_SIZE 512

int main() {
    int sockfd;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket allocation failed");
        exit(EXIT_FAILURE);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr);

    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection to evaluation engine failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    printf("--- Connected to Online Examination Portal ---\n");

    // 1. Interactive Authentication Interface Loop
    int logged_in = 0;
    while (!logged_in) {
        char username[50], password[50];
        printf("\n--- Login Screen ---\n");
        printf("Username: ");
        scanf("%s", username);
        printf("Password: ");
        scanf("%s", password);

        memset(buffer, 0, BUFFER_SIZE);
        snprintf(buffer, BUFFER_SIZE, "%s %s", username, password);
        send(sockfd, buffer, strlen(buffer), 0);

        memset(buffer, 0, BUFFER_SIZE);
        recv(sockfd, buffer, BUFFER_SIZE, 0);

        if (strcmp(buffer, "AUTH_SUCCESS") == 0) {
            printf("[!] Authentication Successful!\n");
            logged_in = 1;
        } else {
            printf("[X] Invalid Credentials. Please access again.\n");
        }
    }

    // 2. Exam Execution Engine Loop
    printf("\n--- Examination Started. Answer all questions below. ---\n");
    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        int n = recv(sockfd, buffer, BUFFER_SIZE, 0);
        if (n <= 0) break;

        // Check if final stream response string contains final calculated scorecard
        if (strncmp(buffer, "SCORE:", 6) == 0) {
            printf("\n==================================\n");
            printf("       EXAM RESULTS SUMMARY       \n");
            printf("==================================\n");
            printf(" Your Final Score: %s\n", buffer + 7);
            printf("==================================\n");
            break;
        }

        // Print incoming question details
        printf("\n%s\n", buffer);
        
        char choice[10];
        printf("Your Choice (A/B/C/D): ");
        scanf("%s", choice);

        // Forward choice input verification string packet to backend matching evaluation tracking bounds
        send(sockfd, choice, 1, 0);
    }

    close(sockfd);
    printf("Connection terminated. Goodbye!\n");
    return 0;
}
