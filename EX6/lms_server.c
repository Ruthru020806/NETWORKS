#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <arpa/inet.h>

#define SERVER_PORT 5001
#define MAX_BOOKS 100

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

Book database[MAX_BOOKS];
int total_books = 0;

void init_database() {
    strcpy(database[0].id, "101"); strcpy(database[0].title, "The Hobbit"); strcpy(database[0].author, "J.R.R. Tolkien"); database[0].available = 1;
    strcpy(database[1].id, "102"); strcpy(database[1].title, "1984"); strcpy(database[1].author, "George Orwell"); database[1].available = 1;
    strcpy(database[2].id, "103"); strcpy(database[2].title, "To Kill a Mockingbird"); strcpy(database[2].author, "Harper Lee"); database[2].available = 0;
    strcpy(database[3].id, "104"); strcpy(database[3].title, "The Great Gatsby"); strcpy(database[3].author, "F. Scott Fitzgerald"); database[3].available = 1;
    total_books = 4;
}

void log_activity(struct sockaddr_in *client_addr, const char *action, const char *details, const char *status) {
    FILE *log_file = fopen("library_server.log", "a");
    time_t now = time(NULL);
    char *timestamp = ctime(&now);
    timestamp[strlen(timestamp) - 1] = '\0'; 

    char client_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(client_addr->sin_addr), client_ip, INET_ADDRSTRLEN);
    int client_port = ntohs(client_addr->sin_port);

    fprintf(stdout, "[%s] Client %s:%d | Action: %s | Details: %s | Status: %s\n", 
            timestamp, client_ip, client_port, action, details, status);
            
    if (log_file) {
        fprintf(log_file, "[%s] Client %s:%d | Action: %s | Details: %s | Status: %s\n", 
                timestamp, client_ip, client_port, action, details, status);
        fclose(log_file);
    }
}

void process_packet(RequestPacket *req, ResponsePacket *res) {
    res->seq = req->seq;
    res->book_count = 0;
    memset(res->message, 0, sizeof(res->message));

    if (req->action == ACTION_VIEW) {
        strcpy(res->status, "SUCCESS");
        for (int i = 0; i < total_books; i++) {
            if (database[i].available) {
                res->books[res->book_count++] = database[i];
            }
        }
    } 
    else if (req->action == ACTION_SEARCH) {
        strcpy(res->status, "SUCCESS");
        for (int i = 0; i < total_books; i++) {
            if (strcasecmp(database[i].title, req->query) == 0 || 
                strcasecmp(database[i].author, req->query) == 0 || 
                strcmp(database[i].id, req->query) == 0) {
                res->books[res->book_count++] = database[i];
            }
        }
    } 
    else if (req->action == ACTION_ISSUE) {
        for (int i = 0; i < total_books; i++) {
            if (strcmp(database[i].id, req->book_id) == 0) {
                if (database[i].available) {
                    database[i].available = 0;
                    strcpy(res->status, "SUCCESS");
                    snprintf(res->message, sizeof(res->message), "Book '%s' issued successfully.", database[i].title);
                } else {
                    strcpy(res->status, "FAILED");
                    strcpy(res->message, "Book is already issued.");
                }
                return;
            }
        }
        strcpy(res->status, "FAILED");
        strcpy(res->message, "Book ID not found.");
    } 
    else if (req->action == ACTION_RETURN) {
        for (int i = 0; i < total_books; i++) {
            if (strcmp(database[i].id, req->book_id) == 0) {
                if (!database[i].available) {
                    database[i].available = 1;
                    strcpy(res->status, "SUCCESS");
                    snprintf(res->message, sizeof(res->message), "Book '%s' returned successfully.", database[i].title);
                } else {
                    strcpy(res->status, "FAILED");
                    strcpy(res->message, "Book was not checked out.");
                }
                return;
            }
        }
        strcpy(res->status, "FAILED");
        strcpy(res->message, "Book ID not found.");
    }
}

int main() {
    int sockfd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);
    
    int last_seen_seq = -1;
    ResponsePacket cached_response;

    init_database();

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(SERVER_PORT);

    if (bind(sockfd, (const struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    printf("Library Server running reliably on UDP port %d...\n", SERVER_PORT);

    while (1) {
        RequestPacket req;
        int n = recvfrom(sockfd, &req, sizeof(RequestPacket), 0, (struct sockaddr *)&client_addr, &addr_len);
        if (n < 0) continue;

        if (req.seq == last_seen_seq) {
            printf("Duplicate request detected (Seq: %d). Resending cached ACK.\n", req.seq);
            sendto(sockfd, &cached_response, sizeof(ResponsePacket), 0, (struct sockaddr *)&client_addr, addr_len);
            continue;
        }

        ResponsePacket res;
        process_packet(&req, &res);
        
        char details[60];
        if(req.action == ACTION_ISSUE || req.action == ACTION_RETURN) {
            snprintf(details, sizeof(details), "ID: %s", req.book_id);
        } else {
            snprintf(details, sizeof(details), "Query: %s", req.query);
        }
        
        log_activity(&client_addr, (req.action == ACTION_VIEW) ? "VIEW" : (req.action == ACTION_SEARCH) ? "SEARCH" : (req.action == ACTION_ISSUE) ? "ISSUE" : "RETURN", details, res.status);

        last_seen_seq = req.seq;
        cached_response = res;

        sendto(sockfd, &res, sizeof(ResponsePacket), 0, (struct sockaddr *)&client_addr, addr_len);
    }

    close(sockfd);
    return 0;
}
