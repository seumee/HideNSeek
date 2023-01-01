#include "myNP.h"

void error_handling(char *message) {
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}

// return value: 1: success, 0: nothing to read, -1: socket disconnect
int readMsg(int s, message* msg) {
    int recv;
    char* ptr = (char*)&msg->header;
    int howMuch = sizeof(stHeader);

    while (1) {
        recv = read(s, ptr, howMuch);
        if(recv == 0) return -1;
        else if(recv == -1) {
            if(errno != EAGAIN) return -1;
            else {
                if(ptr == (char *)&msg->header) return 0;
                else ;  // retry
            }
        }
        else {
            howMuch -= recv;
            ptr += recv;
            if (howMuch == 0)
            {
                if (msg->header.bodyLen == 0) return 1;
                if(ptr > (char *)&msg->body) return 1;
                else howMuch = msg->header.bodyLen;
            }
        }
    }
}

// return value: 1: success, 0: timeout, -1: socket disconnect
//timeLimit: (ms)
int writeMsg(int s, message* msg, int timeLimit) {
    int send;
    char* ptr = (char*)&msg->header;
    int howMuch = msg->header.bodyLen + sizeof(stHeader);
    int totalTime = 0;
    int sleepTime = 1*1000;

    while (totalTime <= (timeLimit*1000)) {
        send = write(s, ptr, howMuch);
        if(send == -1) {
            if(errno == EAGAIN) {
                totalTime += sleepTime;
                usleep(sleepTime);
            }
            else if(errno == EWOULDBLOCK) {
                totalTime += sleepTime;
                usleep(sleepTime);
            }
            else {
                printf("disconnect at writeMSG, errno:%s\n", strerror(errno));
                return -1;
            }
        }
        else {
            howMuch -= send;
            ptr += send;
            if (howMuch == 0)
            {
                if (msg->header.bodyLen == 0) return 1;
                else if(ptr > (char *)&msg->body) return 1;
            }
        }
    }
    printf("time limit over. write fail.\n");
    return -1;
}

void setHeader(stHeader *header, int msgType, int bodyLen) {
    header->msgType = msgType;
    header->bodyLen = bodyLen;
}

//cliIdx -1: include messege sender
//return value: success: 1, fail: -1
int broadcastMsg(message *msg, int roomIdx, int cliIdx) {
    for (int i = 0; i < _gameRoom[roomIdx].userCount; i++)
    {
        int target_userIdx = _gameRoom[roomIdx].user[i];
        if(cliIdx != -1) {
            if (target_userIdx != cliIdx) {
                int sendRet = writeMsg(_client[target_userIdx].socket, msg, 500);
                if(sendRet == -1) return -1;
            }
        } else {
            int sendRet = writeMsg(_client[target_userIdx].socket, msg, 500);
            if(sendRet == -1) return -1;
        }
    }
    return 1;
}

//find user as nickname in specific room
//return value success: cliIdx of corresponding user nickname fail: -1
int findUserByName(int roomIdx, char *nickname) {
    stGameRoom *curRoom = &_gameRoom[roomIdx];
    for(int i = 0; i < curRoom->userCount; i++) {
        cli_sock *tmpC = &_client[curRoom->user[i]];
        if(strcmp(tmpC->nickname, nickname) == 0) return curRoom->user[i];
    }
    return -1;
}
