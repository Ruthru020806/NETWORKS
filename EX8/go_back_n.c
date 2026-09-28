#include <stdio.h>
#include <stdlib.h>

int main() {
    int window_size;
    int total_frames;
    int drop_frame;

    printf("--- Go-Back-N Protocol Simulation ---\n");
    printf("Enter window size: ");
    if (scanf("%d", &window_size) != 1) return 1;
    
    printf("Enter total number of frames to send: ");
    if (scanf("%d", &total_frames) != 1) return 1;

    printf("Enter the frame number to simulate loss/corruption (1 to %d, or 0 for none): ", total_frames);
    if (scanf("%d", &drop_frame) != 1) return 1;

    int total_transmissions = 0;
    int next_frame_to_ack = 1; // Tracks the frame the receiver expects next
    int current_send = 1;      // Tracks the next frame to send initially

    printf("\n--- Starting Data Transmission Simulation ---\n\n");

    while (next_frame_to_ack <= total_frames) {
        printf("Current Window Base: Frame %d\n", next_frame_to_ack);
        printf("Sending frames in current window: [ ");
        
        // 1. Send all frames available in the current window limit
        int window_limit = next_frame_to_ack + window_size - 1;
        if (window_limit > total_frames) {
            window_limit = total_frames;
        }

        for (int i = next_frame_to_ack; i <= window_limit; i++) {
            printf("%d ", i);
            total_transmissions++;
        }
        printf("]\n");

        // 2. Process Acknowledgments frame by frame
        int drop_triggered = 0;
        for (int i = next_frame_to_ack; i <= window_limit; i++) {
            if (i == drop_frame) {
                printf(" -> [!] Frame %d was LOST/CORRUPTED during transmission.\n", i);
                drop_triggered = 1;
                // Disabling drop_frame so it doesn't trigger repeatedly on retransmission
                drop_frame = -1; 
                break;
            } else {
                printf(" -> [✓] Frame %d successfully acknowledged (ACK received).\n", i);
                next_frame_to_ack++;
            }
        }

        // 3. Handle Go-Back-N Timeout Condition
        if (drop_triggered) {
            printf(" -> [!] Timeout occurred! Receiver discarded out-of-order data.\n");
            printf(" -> [GBN] Rolling back. Retransmitting all unacknowledged window frames...\n\n");
        } else {
            printf("\n");
        }
    }

    // 4. Print Transmission Analysis Statistics
    printf("==========================================\n");
    printf("          TRANSMISSION ANALYSIS          \n");
    printf("==========================================\n");
    printf(" Successful Data Frames Delivered : %d\n", total_frames);
    printf(" Total Frame Transmissions Attempted: %d\n", total_transmissions);
    
    double overhead = (double)(total_transmissions - total_frames) / total_frames * 100;
    printf(" Protocol Retransmission Overhead   : %.2f%%\n", overhead);
    printf("==========================================\n");

    return 0;
}
