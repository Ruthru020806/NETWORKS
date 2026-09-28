#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 6000

typedef struct {
    double num1;
    double num2;
    char op; 
} CalcRequest;

typedef struct {
    double result;
    int status;       
    char error_msg[64];
} CalcResponse;

int main() {
    int sockfd;
    struct sockaddr_in server_addr;
    socklen_t addr_len = sizeof(server_addr);

    // Create UDP Socket
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr);

    printf("--- Welcome to the UDP Calculator Interface ---\n");

    while (1) {
        CalcRequest req;
        CalcResponse res;
        char exit_check[10];

        printf("\nEnter operation (+, -, *, /) or 'q' to exit: ");
        fgets(exit_check, sizeof(exit_check), stdin);
        
        // Strip trailing newline character
        exit_check[strcspn(exit_check, "\n")] = 0;

        if (strcmp(exit_check, "q") == 0 || strcmp(exit_check, "Q") == 0) {
            printf("Exiting calculator application.\n");
            break;
        }

        if (strlen(exit_check) != 1 || (exit_check[0] != '+' && exit_check[0] != '-' && exit_check[0] != '*' && exit_check[0] != '/')) {
            printf("Invalid selection choice. Please try again.\n");
            continue;
        }
        req.op = exit_check[0];

        printf("Enter first number: ");
        if (scanf("%lf", &req.num1) != 1) {
            printf("Invalid numeric input.\n");
            while (getchar() != '\n'); // Clear stream buffer
            continue;
        }

        printf("Enter second number: ");
        if (scanf("%lf", &req.num2) != 1) {
            printf("Invalid numeric input.\n");
            while (getchar() != '\n'); 
            continue;
        }
        while (getchar() != '\n'); // Consume remaining structural newline

        // Forward payload packets to destination server mapping
        sendto(sockfd, &req, sizeof(CalcRequest), 0, (const struct sockaddr *)&server_addr, addr_len);

        // Await computation results back
        int n = recvfrom(sockfd, &res, sizeof(CalcResponse), 0, (struct sockaddr *)&server_addr, &addr_len);
        if (n >= 0) {
            if (res.status == 0) {
                printf("\n Result received from Server: %g\n", res.result);
            } else {
                printf("\n Server Processing Error: %s\n", res.error_msg);
            }
        } else {
            printf("\n Network error: Failed to receive data from server.\n");
        }
    }

    close(sockfd);
    return 0;
}
