#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/sysinfo.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <signal.h>
#include "protocol.h"

#define LOG_FILE "audit_trail.log"

// Hardcoded verification target stores
const char* AUTH_USER = "admin";
const char* AUTH_PASS = "stack77";

// Thread & Process safe transaction logger 
void record_transaction(const char* ip, int port, const char* user, const char* message) {
    FILE *log = fopen(LOG_FILE, "a");
    if (!log) return;

    time_t raw_time = time(NULL);
    struct tm *info = localtime(&raw_time);
    char stamp[24];
    strftime(stamp, sizeof(stamp), "%Y-%m-%d %H:%M:%S", info);

    fprintf(log, "[%s] [%s:%d] User(%s): %s\n", 
            stamp, ip, port, (user && strlen(user) > 0) ? user : "UNREGISTERED", message);
    fclose(log);
}

// Clean up terminated worker processes automatically to prevent resource leaks
void reap_zombies(int sig) {
    int saved_errno = errno;
    while (waitpid(-1, NULL, WNOHANG) > 0);
    errno = saved_errno;
}

// Isolated runtime instance loop reserved for active authenticated users
void run_session(int socket_fd, const char* ip, int port) {
    packet_t packet;
    char session_user[CRED_SIZE] = {0};
    int is_authenticated = 0;

    while (1) {
        // Enforce strict protocol sizing reads
        ssize_t bytes = recv(socket_fd, &packet, sizeof(packet_t), 0);
        if (bytes <= 0) {
            record_transaction(ip, port, session_user, "Remote link dropped abruptly.");
            break;
        }

        // Firewall Gatekeeper Constraint Block
        if (!is_authenticated && packet.type != CMD_AUTH) {
            packet.status = STATUS_AUTH_ERR;
            strcpy(packet.data, "Access violation. Authenticate first.");
            send(socket_fd, &packet, sizeof(packet_t), 0);
            continue;
        }

        switch (packet.type) {
            case CMD_AUTH:
                if (strcmp(packet.username, AUTH_USER) == 0 && strcmp(packet.password, AUTH_PASS) == 0) {
                    is_authenticated = 1;
                    strncpy(session_user, packet.username, CRED_SIZE);
                    packet.status = STATUS_OK;
                    snprintf(packet.data, BUFFER_SIZE, "Authentication validated. Welcome %s.", session_user);
                    
                    printf("[AUTH SUCCESS] Client %s:%d verified as '%s'\n", ip, port, session_user);
                    record_transaction(ip, port, session_user, "Successful console profile login.");
                } else {
                    packet.status = STATUS_AUTH_ERR;
                    strcpy(packet.data, "Invalid credential parameters rejected.");
                    record_transaction(ip, port, packet.username, "Failed authentication tentativa.");
                }
                send(socket_fd, &packet, sizeof(packet_t), 0);
                break;

            case CMD_DATETIME: {
                time_t current = time(NULL);
                packet.status = STATUS_OK;
                snprintf(packet.data, BUFFER_SIZE, "Server Time: %s", ctime(&current));
                record_transaction(ip, port, session_user, "Queried system date/time parameters.");
                send(socket_fd, &packet, sizeof(packet_t), 0);
                break;
            }

            case CMD_SYSINFO: {
                struct sysinfo sys;
                if (sysinfo(&sys) == 0) {
                    packet.status = STATUS_OK;
                    snprintf(packet.data, BUFFER_SIZE,
                             "Uptime: %ld sec | Total RAM: %lu MB | Active Tasks: %d",
                             sys.uptime, sys.totalram / (1024 * 1024), sys.procs);
                } else {
                    packet.status = STATUS_IO_ERR;
                    strcpy(packet.data, "Failed to inspect operational metrics.");
                }
                record_transaction(ip, port, session_user, "Inspected host physical engine states.");
                send(socket_fd, &packet, sizeof(packet_t), 0);
                break;
            }

            case CMD_DOWNLOAD: {
                // Prepend dot-slash sequence securely preventing local root file breakouts
                char file_path[PATH_SIZE + 4];
                snprintf(file_path, sizeof(file_path), "./%s", packet.filename);

                FILE *target = fopen(file_path, "rb");
                if (!target) {
                    packet.status = STATUS_NOT_FOUND;
                    snprintf(packet.data, BUFFER_SIZE, "Resource '%s' unavailable.", packet.filename);
                    send(socket_fd, &packet, sizeof(packet_t), 0);
                    record_transaction(ip, port, session_user, "Download failure: Resource missing.");
                } else {
                    record_transaction(ip, port, session_user, "Began downloading server assets.");
                    while (!feof(target)) {
                        packet.payload_len = fread(packet.data, 1, BUFFER_SIZE, target);
                        packet.status = STATUS_OK;
                        send(socket_fd, &packet, sizeof(packet_t), 0);
                    }
                    fclose(target);
                    
                    // Transmission Terminus Token
                    packet.payload_len = 0;
                    send(socket_fd, &packet, sizeof(packet_t), 0);
                }
                break;
            }

            case CMD_UPLOAD: {
                char output_path[PATH_SIZE + 16];
                snprintf(output_path, sizeof(output_path), "./srv_storage_%s", packet.filename);

                FILE *dest = fopen(output_path, "wb");
                if (!dest) {
                    packet.status = STATUS_IO_ERR;
                    strcpy(packet.data, "Write transaction authorization fault.");
                    send(socket_fd, &packet, sizeof(packet_t), 0);
                    break;
                }

                record_transaction(ip, port, session_user, "Began streaming upload data to disk.");
                while (1) {
                    if (packet.payload_len == 0) break; // Terminus sequence code hit
                    fwrite(packet.data, 1, packet.payload_len, dest);

                    if (recv(socket_fd, &packet, sizeof(packet_t), 0) <= 0) break;
                }
                fclose(dest);

                packet.status = STATUS_OK;
                strcpy(packet.data, "Asset deployed securely to host filesystem.");
                send(socket_fd, &packet, sizeof(packet_t), 0);
                record_transaction(ip, port, session_user, "Completed file deployment update.");
                break;
            }

            case CMD_CLOSE:
                packet.status = STATUS_OK;
                strcpy(packet.data, "Session terminated cleanly. Goodbye.");
                send(socket_fd, &packet, sizeof(packet_t), 0);
                record_transaction(ip, port, session_user, "Logged out voluntarily.");
                return;

            default:
                packet.status = STATUS_UNKNOWN_CMD;
                strcpy(packet.data, "Command layout parameters unrecognized.");
                send(socket_fd, &packet, sizeof(packet_t), 0);
                break;
        }
    }
}

int main() {
    int server_socket, client_socket;
    struct sockaddr_in host_cfg, client_cfg;
    socklen_t length = sizeof(client_cfg);
    int reuse_flag = 1;

    // Register OS signal processing hooks to kill process ghosts
    struct sigaction action_struct;
    action_struct.sa_handler = reap_zombies;
    sigemptyset(&action_struct.sa_mask);
    action_struct.sa_flags = SA_RESTART;
    sigaction(SIGCHLD, &action_struct, NULL);

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("Engine initialization fault");
        exit(EXIT_FAILURE);
    }

    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &reuse_flag, sizeof(reuse_flag));

    memset(&host_cfg, 0, sizeof(host_cfg));
    host_cfg.sin_family = AF_INET;
    host_cfg.sin_addr.s_addr = INADDR_ANY;
    host_cfg.sin_port = htons(SERVER_PORT);

    if (bind(server_socket, (struct sockaddr*)&host_cfg, sizeof(host_cfg)) < 0) {
        perror("Address binding registration failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    if (listen(server_socket, 15) < 0) {
        perror("Operational backlog routing error");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    printf("[SERVER ONLINE] Listening for transactions on Port %d...\n", SERVER_PORT);

    while (1) {
        client_socket = accept(server_socket, (struct sockaddr*)&client_cfg, &length);
        if (client_socket < 0) {
            if (errno == EINTR) continue;
            perror("Handshaking registration drop event");
            continue;
        }

        char remote_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_cfg.sin_addr, remote_ip, INET_ADDRSTRLEN);
        int remote_port = ntohs(client_cfg.sin_port);

        // Multiprocessing Concurrent Dispatcher Fork Engine
        pid_t process_id = fork();
        if (process_id == 0) { 
            // Worker Process Execution Environment Context
            close(server_socket); 
            run_session(client_socket, remote_ip, remote_port);
            close(client_socket);
            exit(EXIT_SUCCESS);
        }
        
        // Parent context structural cleanup responsibilities release loop
        close(client_socket); 
    }

    close(server_socket);
    return 0;
}
