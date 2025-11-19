#ifndef PACKET_H
#define PACKET_H

typedef struct {
    int seq_num;
    int size;
    int is_last;
    char data[1024];
} Packet;

typedef struct {
    int ack_num;
} AckPacket;

#endif