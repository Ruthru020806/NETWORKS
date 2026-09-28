#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/time.h>

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 5001
#define MAX_BOOKS 100
#define TIMEOUT_SEC 2
#define MAX_RETRIES 5

typedef enum {
    ACTION_VIEW,
    ACTION_SEARCH,
    ACTION_ISSUE,
    ACTION_RETURN
} ActionType;

typedef struct {
    char id[10];
    char title[50];
    char author[50];
    int available; 
} Book;

typedef struct {
    int seq;
    ActionType action;
    char book_id[10];
    char query[50];
} RequestPacket;

typedef struct {
    int seq;
    char status[10]; 
    char message[100];
    int book_count;
    Book books[MAX_BOOKS]; 
} ResponsePacket;

int global_seq = 0;

int send_reliable_request(int sockfd, struct sockaddr_in *server_addr, RequestPacket *req, ResponsePacket *res) {
    global_seq++;
    req->seq = global_seq;
    socklen_t addr_len = sizeof(*server_addr);
    int attempts = 0;

    while (attempts < MAX_RETRIES) {
        sendto(sockfd, req, sizeof(RequestPacket), 0, (const struct sockaddr *)server_addr, addr_len);

        int n = recvfrom(sockfd, res, sizeof(ResponsePacket), 0, (struct sockaddr *)server_addr, &addr_len);
        if (n >= 0) {
            if (res->seq == global_seq) {
                return 1; 
            }
        } else {
            attempts++;
            printf(" Timeout! No response for sequence %d. Retrying (%d/%d)...\n", global_seq, attempts, MAX_RETRIES);
        }
    }
    printf(" Error: Server is unreachable. Max retries reached.\n");
    return 0;
}

void print_book_catalog(ResponsePacket *res) {
    if (res->book_count == 0) {
        printf("No records matching request found.\n");
        return;
    }
    printf("\n%-6s | %-25s | %-20s | %-10s\n", "ID", "Book Title", "Author", "Status");
    printf("----------------------------------------------------------------------\n");
    for (int i = 0; i < res->book_count; i++) {
        printf("%-6s | %-25s | %-20s | %-10s\n", 
               res->books[i].id, res->books[i].title, res->books[i].author, 
               res->books[i].available ? "Available" : "Issued");
    }
    printf("\n");
}

int main() {
    int sockfd;
    struct sockaddr_in server_addr;

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    struct timeval tv;
    tv.tv_sec = TIMEOUT_SEC;
    tv.tv_usec = 0;
    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
        perror("Failed to set socket timeout");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr);

    printf("--- Welcome to the Reliable UDP Library System ---\n");

    while (1) {
        printf("1. View Available Books\n");
        printf("2. Search for a Book\n");
        printf("3. Issue a Book\n");
        printf("4. Return a Book\n");
        printf("5. Exit System\n");
        printf("Select an option (1-5): ");
        
        int choice;
        if (scanf("%d", &choice) != 1) {
            while (getchar() != '\n'); 
            continue;
        }
        while (getchar() != '\n'); 

        RequestPacket req;
        ResponsePacket res;
        memset(&req, 0, sizeof(req));

        if (choice == 1) {
            req.action = ACTION_VIEW;
            if (send_reliable_request(sockfd, &server_addr, &req, &res) && strcmp(res.status, "SUCCESS") == 0) {
                print_book_catalog(&res);
            }
        } 
        else if (choice == 2) {
            req.action = ACTION_SEARCH;
            printf("Enter Search Query (Title/Author/ID): ");
            fgets(req.query, sizeof(req.query), stdin);
            req.query[strcspn(req.query, "\n")] = 0; 

            if (send_reliable_request(sockfd, &server_addr, &req, &res) && strcmp(res.status, "SUCCESS") == 0) {
                print_book_catalog(&res);
            }
        } 
        else if (choice == 3) {
            req.action = ACTION_ISSUE;
            printf("Enter Book ID to issue: ");
            fgets(req.book_id, sizeof(req.book_id), stdin);
            req.book_id[strcspn(req.book_id, "\n")] = 0;

            if (send_reliable_request(sockfd, &server_addr, &req, &res)) {
                printf("\nServer Response: %s\n\n", res.message);
            }
        } 
        else if (choice == 4) {
            req.action = ACTION_RETURN;
            printf("Enter Book ID to return: ");
            fgets(req.book_id, sizeof(req.book_id), stdin);
            req.book_id[strcspn(req.book_id, "\n")] = 0;

            if (send_reliable_request(sockfd, &server_addr, &req, &res)) {
                printf("\nServer Response: %s\n\n", res.message);
            }
        } 
        else if (choice == 5) {
            printf("Exiting system window.\n");
            break;
        } 
        else {
            printf("Invalid selection choice. Try again.\n");
        }
    }

    close(sockfd);
    return 0;
}
