#ifndef PROTOCOL_H
#define PROTOCOL_H

#define PORT 9090
#define BUFFER_SIZE 1024
#define CRED_SIZE 32
#define FILENAME_SIZE 256

// Menu Service Commands
typedef enum {
    CMD_AUTH = 1,
    CMD_UPLOAD,
    CMD_DOWNLOAD,
    CMD_DATETIME,
    CMD_SYSINFO,
    CMD_EXIT
} command_t;

// Standard Response Status
typedef enum {
    STATUS_SUCCESS = 0,
    STATUS_AUTH_FAIL,
    STATUS_FILE_NOT_FOUND,
    STATUS_ERROR
} status_t;

// Unified Data Packet Structure
typedef struct {
    uint32_t command;       // Type of service requested (command_t)
    uint32_t status;        // Server response status (status_t)
    uint64_t data_len;      // Length of raw payload inside buffer
    char username[CRED_SIZE];
    char password[CRED_SIZE];
    char filename[FILENAME_SIZE];
    char buffer[BUFFER_SIZE]; // Cargo payload area
} packet_t;

#endif
