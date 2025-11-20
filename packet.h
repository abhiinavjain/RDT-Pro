#ifndef PACKET_H
#define PACKET_H

typedef struct{
    int seqnum;
    int size;
    int islast;
    char data[1024];
} Packet;

typedef struct {
    int ackNum;

} AckPacket;

#endif