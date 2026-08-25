#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 8082
#define SERVER_IP "127.2.2.3"   /* change if server runs on another machine */

int main() {
    int sock_fd;
    struct sockaddr_in server_addr;

    /* 1. Create socket */
    sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    /* 2. Prepare server address structure */
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr) <= 0) {
        perror("Invalid server address");
        close(sock_fd);
        exit(EXIT_FAILURE);
    }

    /* 3. Connect to the server */
    if (connect(sock_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection to server failed");
        close(sock_fd);
        exit(EXIT_FAILURE);
    }

    printf("Connected to server.\n");

    /* 4. Receive the array size first */
    int size_net;
    if (recv(sock_fd, &size_net, sizeof(size_net), 0) <= 0) {
        perror("Failed to receive array size");
        close(sock_fd);
        exit(EXIT_FAILURE);
    }
    int arr_size = ntohl(size_net);

    /* 5. Allocate memory and receive the array itself */
    int *net_arr = malloc(arr_size * sizeof(int));
    int total_bytes = arr_size * sizeof(int);
    int received = 0;

    while (received < total_bytes) {
        int n = recv(sock_fd, ((char *)net_arr) + received, total_bytes - received, 0);
        if (n <= 0) {
            perror("Failed to receive array data");
            free(net_arr);
            close(sock_fd);
            exit(EXIT_FAILURE);
        }
        received += n;
    }

    /* 6. Convert from network byte order, print elements, and calculate sum */
    int sum = 0;
    printf("Array received from server: ");
    for (int i = 0; i < arr_size; i++) {
        int value = ntohl(net_arr[i]);
        printf("%d ", value);
        sum += value;
    }
    printf("\n");

    printf("Sum of array elements = %d\n", sum);

    /* 7. Send the sum back to the server (in network byte order) */
    int sum_net = htonl(sum);
    send(sock_fd, &sum_net, sizeof(sum_net), 0);

    /* 8. Cleanup */
    free(net_arr);
    close(sock_fd);
    return 0;
}
