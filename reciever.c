#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "packet.h"

void error(char *msg) {
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <port> <output_file>\n", argv[0]);
        exit(1);
    }

    int port = atoi(argv[1]);
    int sockfd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
        error("ERROR opening socket");

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
        error("ERROR on binding");

    FILE *fp = fopen(argv[2], "wb");
    if (fp == NULL) error("ERROR opening file");

    printf("Receiver listening on port %d...\n", port);

    int expected_seq = 0;

    while (1) {
        Packet pkt;
        int n = recvfrom(sockfd, &pkt, sizeof(pkt), 0, (struct sockaddr *)&client_addr, &addr_len);
        if (n < 0) error("ERROR in recvfrom");

        printf("Received Packet SEQ: %d (Expected: %d)\n", pkt.seq_num, expected_seq);

        AckPacket ack;
        ack.ack_num = pkt.seq_num;
        sendto(sockfd, &ack, sizeof(ack), 0, (struct sockaddr *)&client_addr, addr_len);

        if (pkt.seq_num == expected_seq) {
            if (pkt.is_last) {
                printf("End of file marker received.\n");
                break;
            }
            fwrite(pkt.data, 1, pkt.size, fp);
            expected_seq = 1 - expected_seq;
        }
    }

    fclose(fp);
    close(sockfd);
    return 0;
}