#ifndef MYNP_H
#define MYNP_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <time.h>
#include <errno.h>
#include <signal.h>
#include <sys/select.h>
#include "myMessage.h"
#include "myRoom.h"

#define MAX_CONNECT 128

void setHeader(stHeader *header, int msgType, int bodyLen);
void error_handling(char *message);
void fullRoom(message *msg, int s);
void makeMsg(message *msg, int id, int msgType, void* content);
int readMsg(int s, message* msg);
int writeMsg(int s, message* msg, int timeLimit);
int broadcastMsg(message *msg, int roomIdx, int includeSender);
int findUserByName(int roomIdx, char *nickname);

typedef struct client_socket {
    int socket;         // 0: NULL
    //int id;             // 0: NULL
    char nickname[NICKNAME_SIZE];   // NULL string : NULL
    int room_id;        // 0: NULL
} cli_sock;

extern cli_sock _client[MAX_CONNECT];

#endif
