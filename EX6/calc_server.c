#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define SERVER_PORT 6000

// Request payload structure sent by the client
typedef struct {
    double num1;
    double num2;
    char op; // '+', '-', '*', '/'
} CalcRequest;

// Response payload structure sent by the server
typedef struct {
    double result;
    int status;       // 0 = Success, -1 = Error/Validation Failure
    char error_msg[64];
} CalcResponse;

int main() {
    int sockfd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);

    // Create UDP Socket
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(SERVER_PORT);

    // Bind socket to server port
    if (bind(sockfd, (const struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    printf("Calculator Server is running on UDP port %d...\n", SERVER_PORT);

    while (1) {
        CalcRequest req;
        CalcResponse res;
        memset(&res, 0, sizeof(res));

        // Receive request from client
        int n = recvfrom(sockfd, &req, sizeof(CalcRequest), 0, (struct sockaddr *)&client_addr, &addr_len);
        if (n < 0) continue;

        // Visual server log tracking
        printf("Received request: %g %c %g\n", req.num1, req.op, req.num2);

        // Process request and validate inputs
        res.status = 0; 
        switch (req.op) {
            case '+':
                res.result = req.num1 + req.num2;
                break;
            case '-':
                res.result = req.num1 - req.num2;
                break;
            case '*':
                res.result = req.num1 * req.num2;
                break;
            case '/':
                if (req.num2 == 0.0) {
                    res.status = -1;
                    strcpy(res.error_msg, "Division by zero condition detected!");
                } else {
                    res.result = req.num1 / req.num2;
                }
                break;
            default:
                res.status = -1;
                strcpy(res.error_msg, "Invalid mathematical operator.");
                break;
        }

        // Send computed calculation back to client
        sendto(sockfd, &res, sizeof(CalcResponse), 0, (struct sockaddr *)&client_addr, addr_len);
    }

    close(sockfd);
    return 0;
}
