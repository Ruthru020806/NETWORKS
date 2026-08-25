#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 8082
#define ARR_SIZE 5

int main() {
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);

    /* The array to send to the client */
    int arr[ARR_SIZE] = {10, 20, 30, 40, 50};

    /* 1. Create socket */
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    /* Allow immediate reuse of port after restart */
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    /* 2. Prepare server address structure */
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;   /* listen on all interfaces */
    server_addr.sin_port = htons(PORT);

    /* 3. Bind socket to address and port */
    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    /* 4. Listen for incoming connections (waiting queue size = 5) */
    if (listen(server_fd, 5) < 0) {
        perror("Listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Server is listening on port %d...\n", PORT);

    while (1) {
        /* 5. Accept a client connection */
        client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
        if (client_fd < 0) {
            perror("Accept failed");
            continue;
        }

        printf("Client connected: %s\n", inet_ntoa(client_addr.sin_addr));

        /* 6. First send the size of the array, so the client knows how much to expect */
        int size_net = htonl(ARR_SIZE);
        send(client_fd, &size_net, sizeof(size_net), 0);

        /* 7. Convert each element to network byte order and send the array */
        int net_arr[ARR_SIZE];
        for (int i = 0; i < ARR_SIZE; i++) {
            net_arr[i] = htonl(arr[i]);
        }
        send(client_fd, net_arr, sizeof(net_arr), 0);

        printf("Array sent to client: ");
        for (int i = 0; i < ARR_SIZE; i++) {
            printf("%d ", arr[i]);
        }
        printf("\n");

        /* 8. Receive the sum calculated by the client (optional confirmation) */
        int recv_sum_net;
        int bytes_received = recv(client_fd, &recv_sum_net, sizeof(recv_sum_net), 0);
        if (bytes_received == sizeof(recv_sum_net)) {
            int recv_sum = ntohl(recv_sum_net);
            printf("Sum received back from client: %d\n", recv_sum);
        }

        /* 9. Close connection with this client */
        close(client_fd);
    }

    close(server_fd);
    return 0;
}
