#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/time.h>
#include "packet.h"

void error(char *msg) {
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <ip> <port> <filename>\n", argv[0]);
        exit(1);
    }

    int port = atoi(argv[2]);
    int sockfd;
    struct sockaddr_in server_addr;

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
        error("ERROR opening socket");

    struct timeval tv;
    tv.tv_sec = 2;
    tv.tv_usec = 0;
    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
        error("Error setting timeout");
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    inet_aton(argv[1], &server_addr.sin_addr);

    FILE *fp = fopen(argv[3], "rb");
    if (fp == NULL) error("ERROR opening file");

    Packet pkt;
    int seq = 0;
    int n_read;

    while ((n_read = fread(pkt.data, 1, sizeof(pkt.data), fp)) > 0) {
        pkt.size = n_read;
        pkt.seq_num = seq;
        pkt.is_last = 0;

        while (1) {
            sendto(sockfd, &pkt, sizeof(pkt), 0, (struct sockaddr *)&server_addr, sizeof(server_addr));
            printf("Sent Packet SEQ: %d, Size: %d\n", pkt.seq_num, pkt.size);

            AckPacket ack;
            struct sockaddr_in from_addr;
            socklen_t addr_len = sizeof(from_addr);
            
            int n = recvfrom(sockfd, &ack, sizeof(ack), 0, (struct sockaddr *)&from_addr, &addr_len);

            if (n < 0) {
                printf("!!! Timeout waiting for ACK %d. Retransmitting...\n", seq);
                continue;
            } else if (ack.ack_num == seq) {
                printf("Received ACK: %d\n", ack.ack_num);
                break;
            }
        }
        
        seq = 1 - seq;
    }

    pkt.size = 0;
    pkt.is_last = 1;
    pkt.seq_num = seq;
    sendto(sockfd, &pkt, sizeof(pkt), 0, (struct sockaddr *)&server_addr, sizeof(server_addr));
    printf("File transfer complete.\n");

    fclose(fp);
    close(sockfd);
    return 0;
}