#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include "packet.h"

void error(char *msg){
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[]){
    if(argc != 4){
        fprintf(stderr, argv[0]);
        exit(1);
    }

    WSADATA wsa;
    if(WSAStartup(MAKEWORD(2,2), &wsa) != 0){
        error("WSAStartup failed");
    }

    int port = atoi(argv[2]);
    SOCKET client_socket;

    if((client_socket = socket(AF_INET, SOCK_DGRAM, 0))== INVALID_SOCKET){
        error("Error opening socket");
    }
    DWORD timeout = 2000;
    if (setsockopt(client_socket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout))<0){
        error("Error setting timeout");

    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family= AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(argv[1]);

    FILE *fp = fopen(argv[3], "rb");
    if(fp==NULL){
        error("error opening file");

    }

    Packet pkt; 
    int seq = 0;
    int n_read; 

    while((n_read= fread(pkt.data, 1, sizeof(pkt.data), fp))>0){
        pkt.size = n_read;
        pkt.seqnum = seq; 
        pkt.islast  = 0;

        while(1){
            sendto(client_socket, (char *) &pkt, sizeof(pkt), 0 , (struct sockaddr *)&server_addr, sizeof(server_addr));
            printf("Sent packet seq no : %d", seq);
            AckPacket ack;
            struct sockaddr_in from_addr;
            int addr_len = sizeof(from_addr);

            int n = recvfrom(client_socket, (char*)&ack, sizeof(ack), 0, (struct sockaddr *)&from_addr, &addr_len);

            if (n== SOCKET_ERROR){
                printf("Transmission timeout \n");
                continue;
            }

            else if(ack.ackNum == seq){
                printf("recieved ack %d", ack.ackNum);
                break;

            }

        }

        seq =  1 - seq;
        
        

    }


    pkt.size = 0;

    pkt.islast = 1; 

    pkt.seqnum = seq; 

    sendto(client_socket, (char*)&pkt, sizeof(pkt), 0, (struct sockaddr *)&server_addr, sizeof(server_addr));


    printf("File transfer completed");

    fclose(fp);
    closesocket(client_socket);
    WSACleanup();
    return 0;

     
}