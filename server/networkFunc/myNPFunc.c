#include "myNP.h"

void error_handling(char *message) {
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}

int PopCompleteMsgFromBuf(char *msgBuffer, char** bufferPtr, char* CompleteMsg) {
    //
    int BufDataLen = *bufferPtr-msgBuffer;
    for(int idx = 0; idx < BufDataLen; idx++) {
        if(msgBuffer[idx] == 0) {
            memcpy(CompleteMsg, msgBuffer, idx+1);
            memcpy(msgBuffer, msgBuffer+idx+1, BufDataLen-idx-1);
            *bufferPtr -= (idx+1);
            return 1;
        }
    }
    return 0;
}


#if 0
// 1<return value: complete message & length of string input.    0: not a complete message.     -1: socket disconnect
int readMsg(int s, char* msgBuffer, char** bufferPtr) {
    char buffer[MAX_BUFFER_SIZE];
    int recv;
    int tmp;
    if(*bufferPtr != msgBuffer) tmp = *bufferPtr - msgBuffer;
    while (1) {
        recv = read(s, buffer, MAX_BUFFER_SIZE);    //MAX_BUFFER -> 오버ㅠㅡㄹ로우 위험
        if(recv == 0) return -1;
        else if(recv == -1) {
            if(errno != EAGAIN) return -1;
            //??
            #if 0
            else {
                //if(ptr == (char *)&msg->header) return 0;
                else ;  // retry
            }
            #endif
        }
        else {
            printf("buffer[strlen(buffer)-2]: %c\n", buffer[strlen(buffer)-2]);
            printf("buffer[strlen(buffer)-1]: %c\n", buffer[strlen(buffer)-1]);
            printf("buffer[strlen(buffer)]: %c\n", buffer[strlen(buffer)]);
            printf("%s\n", buffer);
            
            memcpy(*bufferPtr, buffer, recv);
            *bufferPtr += recv;
            if(*(*bufferPtr-1) != '\0') {
                return 0;
            }
            return recv+tmp;
        }
    }
}
#endif

// return value: 1: success, 0: timeout, -1: socket disconnect
//timeLimit: (ms)
int writeMsg(int s, char* jsonStr, int timeLimit) {
    //printf("!!write: %s!!\n", jsonStr);
    int send;
    char* ptr = jsonStr;
    int howMuch = 0;
    while(jsonStr[howMuch] != '\0') {
        if(jsonStr[howMuch++] == '\n') break;
        howMuch++;
    }
    jsonStr[howMuch] = '\n';
    int totalTime = 0;
    int sleepTime = 1*100;
    //printf("write: %s\n", jsonStr);
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
            if (howMuch == 0) {
                return 1;
            }
        }
    }
    printf("time limit over. write fail.\n");
    return -1;
}

//includeSender == -1: include messege sender
//return value: success: 1, fail: -1
int broadcastMsg(char *jsonStr, int roomIdx, int includeSender) {
    char string_copy[MAX_BUFFER_SIZE];
    memset(string_copy, 0, sizeof(string_copy));
    strcpy(string_copy, jsonStr);
    for (int i = 0; i < MAX_ROOM_SIZE; i++)
    {
        if(_gameRoom[roomIdx].user[i] != -1) {
            string_copy[strlen(string_copy)] = '\n';
            int target_userIdx = _gameRoom[roomIdx].user[i];
            if(includeSender != -1) {
                if (target_userIdx != includeSender) {
                    int sendRet = writeMsg(_client[target_userIdx].socket, string_copy, 200);
                    if(sendRet == -1) return -1;
                }
            } else {
                int sendRet = writeMsg(_client[target_userIdx].socket, string_copy, 200);
                if(sendRet == -1) return -1;
            }
        }
    }
    return 1;
}

//find user as nickname in specific room
//return value success: cliIdx of corresponding user nickname. fail: -1
int findUserByName(int roomIdx, char *nickname) {
    stGameRoom *curRoom = &_gameRoom[roomIdx];
    for(int i = 0; i < MAX_ROOM_SIZE; i++) {
        cli_sock *tmpC = &_client[curRoom->user[i]];
        if(strcmp(tmpC->nickname, nickname) == 0) return curRoom->user[i];
    }
    return -1;
}

//find admin's colorIdx(stGameRoom.user[index]) as cliIdx
//return value success: colorIdx of corresponding room admin. fail: -1
int findAdminColorIdxByCliIdx(int cliIdx) {
    int roomIdx = _client[cliIdx].room_id-1;
    stGameRoom *curRoom = &_gameRoom[roomIdx];
    for(int i = 0; i < MAX_ROOM_SIZE; i++) {
        if(curRoom->user[i] == curRoom->admin) return i;
    }
    return -1;
}