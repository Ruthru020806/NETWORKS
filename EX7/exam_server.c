#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <arpa/inet.h>
#include <pthread.h>

#define PORT 7000
#define MAX_QUESTIONS 5
#define BUFFER_SIZE 512

// Structure definitions
typedef struct {
    char username[50];
    char password[50];
} User;

typedef struct {
    int id;
    char text[256];
    char options[128];
    char correct_ans;
} Question;

// Mock Databases
User users[] = {
    {"student1", "pass123"},
    {"student2", "exam2026"},
    {"student3", "secure99"}
};
int num_users = 3;

Question exam_db[MAX_QUESTIONS] = {
    {1, "What is the time complexity of binary search?", "A) O(n) B) O(log n) C) O(n^2) D) O(1)", 'B'},
    {2, "Which protocol works at the Transport layer?", "A) IP B) HTTP C) TCP D) DNS", 'C'},
    {3, "Which of the following is not a C keyword?", "A) volatile B) switch C) dynamic D) sizeof", 'C'},
    {4, "What does HTTP stand for?", "A) High Text Transfer Protocol B) Hypertext Transfer Protocol", 'B'},
    {5, "In C, what is the size of a pointer variable?", "A) 2 bytes B) 4 bytes C) 8 bytes D) Platform dependent", 'D'}
};

// Thread Synchronization Locks
pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t db_mutex = PTHREAD_MUTEX_INITIALIZER;

// Utility function to log actions securely
void server_log(const char *username, const char *action, const char *status) {
    pthread_mutex_lock(&log_mutex);
    FILE *log_file = fopen("exam_server.log", "a");
    time_t now = time(NULL);
    char *timestamp = ctime(&now);
    timestamp[strlen(timestamp) - 1] = '\0'; // Strip newline

    fprintf(stdout, "[%s] User: %s | Action: %s | Status: %s\n", timestamp, username, action, status);
    if (log_file) {
        fprintf(log_file, "[%s] User: %s | Action: %s | Status: %s\n", timestamp, username, action, status);
        fclose(log_file);
    }
    pthread_mutex_unlock(&log_mutex);
}

// Client Handling Routine
void *client_handler(void *arg) {
    int client_sock = *(int *)arg;
    free(arg);
    
    char buffer[BUFFER_SIZE];
    char current_user[50] = "Unauthenticated";
    int authenticated = 0;
    int score = 0;

    // 1. Authentication Phase
    while (!authenticated) {
        memset(buffer, 0, BUFFER_SIZE);
        if (recv(client_sock, buffer, BUFFER_SIZE, 0) <= 0) {
            server_log(current_user, "Disconnection during auth", "CLOSED");
            close(client_sock);
            return NULL;
        }

        char input_user[50], input_pass[50];
        sscanf(buffer, "%s %s", input_user, input_pass);

        pthread_mutex_lock(&db_mutex); // Lock DB read during auth check
        for (int i = 0; i < num_users; i++) {
            if (strcmp(users[i].username, input_user) == 0 && strcmp(users[i].password, input_pass) == 0) {
                authenticated = 1;
                strcpy(current_user, input_user);
                break;
            }
        }
        pthread_mutex_unlock(&db_mutex);

        if (authenticated) {
            send(client_sock, "AUTH_SUCCESS", 12, 0);
            server_log(current_user, "Login Attempt", "SUCCESS");
        } else {
            send(client_sock, "AUTH_FAILED", 11, 0);
            server_log(input_user, "Login Attempt", "FAILED");
        }
    }

    // 2. Examination Phase
    for (int i = 0; i < MAX_QUESTIONS; i++) {
        // Safe access packet construction
        pthread_mutex_lock(&db_mutex);
        Question q = exam_db[i];
        pthread_mutex_unlock(&db_mutex);

        // Send Question details
        memset(buffer, 0, BUFFER_SIZE);
        snprintf(buffer, BUFFER_SIZE, "Q%d: %s\n%s", q.id, q.text, q.options);
        if (send(client_sock, buffer, strlen(buffer), 0) <= 0) {
            server_log(current_user, "Disconnection during exam", "ABORTED");
            close(client_sock);
            return NULL;
        }

        // Receive Answer payload
        memset(buffer, 0, BUFFER_SIZE);
        if (recv(client_sock, buffer, BUFFER_SIZE, 0) <= 0) {
            server_log(current_user, "Disconnection during answer collection", "ABORTED");
            close(client_sock);
            return NULL;
        }

        char student_ans = buffer[0];
        if (student_ans == q.correct_ans || student_ans == (q.correct_ans + 32) || student_ans == (q.correct_ans - 32)) {
            score++;
        }
    }

    // 3. Finalization Phase
    memset(buffer, 0, BUFFER_SIZE);
    snprintf(buffer, BUFFER_SIZE, "SCORE: %d/%d", score, MAX_QUESTIONS);
    send(client_sock, buffer, strlen(buffer), 0);
    
    char score_str[20];
    snprintf(score_str, sizeof(score_str), "%d points", score);
    server_log(current_user, "Exam Completed", score_str);

    close(client_sock);
    return NULL;
}

int main() {
    int server_sock, *new_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);

    if ((server_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket allocation error");
        exit(EXIT_FAILURE);
    }

    int opt = 1;
    setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind target validation failure");
        close(server_sock);
        exit(EXIT_FAILURE);
    }

    if (listen(server_sock, 10) < 0) {
        perror("Listen state failed");
        close(server_sock);
        exit(EXIT_FAILURE);
    }

    printf("Concurrent Examination Server handling requests on TCP Port %d...\n", PORT);

    while (1) {
        int client_fd = accept(server_sock, (struct sockaddr *)&client_addr, &addr_len);
        if (client_fd < 0) continue;

        // Dynamic parameter allocation to avoid context racing among threads
        new_sock = malloc(sizeof(int));
        *new_sock = client_fd;

        pthread_t client_thread;
        if (pthread_create(&client_thread, NULL, client_handler, (void *)new_sock) < 0) {
            perror("Thread generation failure");
            free(new_sock);
            close(client_fd);
        }
        
        // Detach thread to avoid memory leaks upon worker lifecycle end
        pthread_detach(client_thread);
    }

    close(server_sock);
    return 0;
}
