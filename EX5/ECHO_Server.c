#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main() {
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    char buffer[BUFFER_SIZE];
    int opt = 1;

    // 1. Create a TCP socket (IPv4, Stream-based TCP)
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // 2. Set socket options to reuse the address and port instantly after shutdown
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // 3. Configure the server address structure
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY; // Accept connections on any network interface
    server_addr.sin_port = htons(PORT);       // Convert port to network byte order

    // 4. Bind the socket to the specified port and address
    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // 5. Start listening for incoming connections (backlog queue size = 5)
    if (listen(server_fd, 5) < 0) {
        perror("Listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Echo server is running and listening on port %d...\n", PORT);

    // 6. Main server loop to accept and handle client connections sequentially
    while (1) {
        client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
        if (client_fd < 0) {
            perror("Accept failed");
            continue; // Keep server running if an individual connection fails
        }

        // Print connected client's IP and Port details
        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
        printf("Client connected from %s:%d\n", client_ip, ntohs(client_addr.sin_port));

        // 7. Receive and echo data back loop
        ssize_t bytes_received;
        while ((bytes_received = recv(client_fd, buffer, BUFFER_SIZE, 0)) > 0) {
            // Echo the exact bytes back to the client
            ssize_t bytes_sent = send(client_fd, buffer, bytes_received, 0);
            if (bytes_sent < 0) {
                perror("Send failed");
                break;
            }
        }

        // Check connection closing conditions
        if (bytes_received == 0) {
            printf("Client disconnected gracefully.\n");
        } else if (bytes_received < 0) {
            perror("Read error or client dropped connection ungracefully");
        }

        // 8. Clean up client socket for next connection
        close(client_fd);
    }

    // Close the listening socket (Unreachable code in an infinite loop, but good practice)
    close(server_fd);
    return 0;
}
