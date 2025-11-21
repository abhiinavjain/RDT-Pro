#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include "packet.h"
#include <time.h>


void error(char *msg){
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[]){
    if(argc!= 3){
        fprintf(stderr, argv[0]);
        exit(1);

    }

    // argv = reciever.exe 8080 output.txt

    WSADATA wsa;

    if(WSAStartup(MAKEWORD(2,2), &wsa) != 0){
        error("WSAStartup failed");
    }

    int port = atoi(argv[1]);
    SOCKET server_socket;

    struct sockaddr_in server_addr, client_addr;
    int addr_len = sizeof(client_addr);

    if ((server_socket = socket(AF_INET, SOCK_DGRAM, 0))== INVALID_SOCKET){
        error("Error opening socket");
    }

    memset (&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr))== SOCKET_ERROR){
        error("Error on binding");
    }

    FILE *fp = fopen(argv[2], "wb");
    if (fp == NULL) error("ERROR opening file");

    printf("Reciever listening on port %d \n", port);

    int expected_seq = 0;
    
    while(1){
        Packet pkt;
        int n = recvfrom(server_socket, (char*)&pkt, sizeof(pkt), 0, (struct sockaddr *)&client_addr, &addr_len);

        if (n==SOCKET_ERROR){
            error("Error in recvfrom ");
  //          continue;

        }

        // if((rand()%10) <3){
        //     printf("simulating oacket loss %d", pkt.seqnum);
        //     continue;
        // }





        printf("Recieved sequence no: %d", pkt.seqnum );
        AckPacket ack;
        ack.ackNum = pkt.seqnum;

        sendto(server_socket, (char*)&ack, sizeof(ack),0, (struct sockaddr *)&client_addr, addr_len);

        if(pkt.seqnum==expected_seq){
            if(pkt.islast){
                break;
            }
            fwrite(pkt.data, 1, pkt.size, fp);
            expected_seq = 1- expected_seq;
        }


    }


    fclose(fp);

    closesocket(server_socket);

    WSACleanup();

    return 0;







}