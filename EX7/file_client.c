#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <dirent.h>

#define PORT 8000
#define BUFFER_SIZE 1024

// Thread function to handle individual clients
void *handle_client(void *arg) {
    int client_sock = *(int *)arg;
    free(arg); // Free the dynamically allocated socket descriptor memory

    char buffer[BUFFER_SIZE];
    printf("[Server] New client thread assigned to socket descriptor %d.\n", client_sock);

    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        // Read command choice from client
        int read_bytes = recv(client_sock, buffer, BUFFER_SIZE, 0);
        if (read_bytes <= 0) {
            printf("[Server] Client disconnected or read error on socket %d.\n", client_sock);
            break;
        }

        // Command 1: Send list of files
        if (strcmp(buffer, "LIST") == 0) {
            DIR *d;
            struct dirent *dir;
            char list_buffer[BUFFER_SIZE * 4] = ""; 

            d = opendir("."); // Scan current directory
            if (d) {
                while ((dir = readdir(d)) != NULL) {
                    // Skip directory navigation links and regular source/executable binaries
                    if (dir->d_type == DT_REG && strstr(dir->d_name, ".c") == NULL && strstr(dir->d_name, "server") == NULL && strstr(dir->d_name, "client") == NULL) {
                        strcat(list_buffer, dir->d_name);
                        strcat(list_buffer, "\n");
                    }
                }
                closedir(d);
            }
            
            if (strlen(list_buffer) == 0) {
                strcpy(list_buffer, "(No shared files available in server root directory)\n");
            }

            send(client_sock, list_buffer, strlen(list_buffer), 0);
        }
        // Command 2: Download a targeted file
        else if (strncmp(buffer, "GET ", 4) == 0) {
            char *filename = buffer + 4;
            // Basic security: strip trailing newline if present
            filename[strcspn(filename, "\r\n")] = 0;

            printf("[Server] Socket %d requested file: %s\n", client_sock, filename);

            FILE *file = fopen(filename, "rb");
            if (file == NULL) {
                // Inform client file does not exist
                send(client_sock, "ERROR: FILE_NOT_FOUND", 21, 0);
                continue;
            }

            send(client_sock, "START_TRANSFER", 14, 0);
            usleep(10000); // Tiny pause to avoid packet blending

            // Read from file and stream across TCP network in chunks
            int bytes_read;
            while ((bytes_read = fread(buffer, 1, BUFFER_SIZE, file)) > 0) {
                send(client_sock, buffer, bytes_read, 0);
            }
            fclose(file);
            
            // Graceful shutdown indicator on individual socket channel
            close(client_sock);
            printf("[Server] File %s sent successfully. Connection on socket %d closed.\n", filename, client_sock);
            return NULL;
        }
        else if (strcmp(buffer, "EXIT") == 0) {
            printf("[Server] Client on socket %d chose to exit.\n", client_sock);
            break;
        }
    }

    close(client_sock);
    return NULL;
}

int main() {
    int server_sock, *new_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);

    if ((server_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket allocation failed");
        exit(EXIT_FAILURE);
    }

    int opt = 1;
    setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(server_sock);
        exit(EXIT_FAILURE);
    }

    if (listen(server_sock, 10) < 0) {
        perror("Listen failed");
        close(server_sock);
        exit(EXIT_FAILURE);
    }

    printf("Concurrent File Sharing Server listening on TCP Port %d...\n", PORT);

    while (1) {
        int client_fd = accept(server_sock, (struct sockaddr *)&client_addr, &addr_len);
        if (client_fd < 0) continue;

        new_sock = malloc(sizeof(int));
        *new_sock = client_fd;

        pthread_t client_thread;
        if (pthread_create(&client_thread, NULL, handle_client, (void *)new_sock) < 0) {
            perror("Could not create thread");
            free(new_sock);
            close(client_fd);
        }
        
        // Detach the thread so resources auto-clean upon termination
        pthread_detach(client_thread);
    }

    close(server_sock);
    return 0;
}
