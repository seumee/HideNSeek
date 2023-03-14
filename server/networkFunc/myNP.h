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
#include <json-c/json.h>
#include "messageHandler/myMessage.h"
#include "gameRoom/myRoom.h"

#define MAX_CONNECT 128

int json_decode(char* jsonStr, uniMsg* stBody, int cliIdx, int *maxfd);
int json_decode_login_req(json_object* msgObj, stLogin_req* stReq);
int json_encode_login_rsp(stLogin_rsp* stRsp, int cliIdx);
int json_decode_crt_room_req(json_object* msgObj, stCrt_room_req* stReq);
int json_encode_crt_room_rsp(stCrt_room_rsp* stRsp, int cliIdx);
int json_decode_join_room_req(json_object* msgObj, stJoin_room_req* stReq);
int json_encode_join_room_rsp(stJoin_room_rsp* stRsp, int cliIdx);
int json_decode_config_room_rpt(json_object* msgObj, stConfig_room_rpt* stRpt);
int json_decode_game_start_req(json_object* msgObj, stGame_start_req* stReq);
int json_encode_game_start_rsp(stGame_start_rsp* stRsp, int cliIdx);
int json_decode_game_ctrl_rpt(json_object* msgObj, stGame_ctrl_rpt* stRpt);
int json_decode_del_room_req(json_object* msgObj, stDel_room_req* stReq);
int json_encode_del_room_rsp(stDel_room_rsp* stRsp, int cliIdx);
int json_decode_leave_room_req(json_object* msgObj, stLeave_room_req* stReq);
int json_encode_leave_room_rsp(stLeave_room_rsp* stRsp, int cliIdx, int roomId);
int json_decode_handover_admin_rpt(json_object* msgObj, stHandover_admin_rpt* stReq);
int json_decode_kick_user_rpt(json_object* msgObj, stKick_user_rpt* stRpt);
int json_decode_room_list_req(json_object* msgObj, stRoom_list_req* stReq);
int json_encode_room_list_rsp(stRoom_list_rsp* stRsp, int cliIdx);
int json_decode_position_rpt(json_object* msgObj, stPosition_rpt* stReq);
int json_decode_catch_rpt(json_object* msgObj, stCatch_rpt* stReq);
int json_decode_block_rpt(json_object* msgObj, stBlock_rpt* stRpt);
int json_decode_game_end_req(json_object* msgObj, stGame_end_req* stReq);
int json_encode_game_end_rsp(stGame_end_rsp* stRsp, int cliIdx);

//void setHeader(stHeader *header, int msgType, int bodyLen);
//void makeMsg(uniMsg *msg, int id, int msgType, void* content);
void error_handling(char *message);
void fullRoom(uniMsg *msg, int s);
int PopCompleteMsgFromBuf(char *msgBuffer, char** bufferPtr, char* CompleteMsg);    //return: success(1),   not yet or empty buffer(0)
int readMsg(int s, char* msgBuffer, char** bufferPtr);
int writeMsg(int s, char* jsonStr, int timeLimit);
int broadcastMsg(char *jsonStr, int roomIdx, int includeSender);
int findUserByName(int roomIdx, char *nickname);
int findAdminColorIdxByCliIdx(int cliIdx);

typedef struct client_socket {
    int socket;         // 0: NULL
    char nickname[NICKNAME_SIZE];   // NULL string : NULL
    int room_id;        // 0: NULL
    char msgBuffer[MAX_BUFFER_SIZE];
    char* bufferPtr;
} cli_sock;

extern cli_sock _client[MAX_CONNECT];

#endif