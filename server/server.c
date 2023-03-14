#include "networkFunc/myNP.h"

int get_curMaxfd();

cli_sock _client[MAX_CONNECT];
fd_set _mainfds;
int con_count = 0;

int main(int argc, char *argv[]) {
    signal(SIGPIPE, SIG_IGN);
    int serv_sock;

    fd_set checkfds;
    int countProc;
    int maxfd = 0;

    struct sockaddr_in serv_addr;
    struct sockaddr_in cli_addr;
    socklen_t cli_addr_size = sizeof(cli_addr);
    int option;

    int id = 1;
    int tmp_sock;

    int isWork;
    char subStr[MAX_BUFFER_SIZE];

    char buff[MSG_LENGTH];
    uniMsg msg;
    for(int i = 0; i < MAX_CONNECT; i ++) _client[i].socket = -1;

    serv_sock = socket(PF_INET, SOCK_STREAM, 0);
    option = 1;
    setsockopt(serv_sock, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));
    fcntl(serv_sock, F_SETFL, O_NONBLOCK);
    if (serv_sock == -1) error_handling("socket error");
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family=AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(atoi(argv[1]));
    if (bind(serv_sock, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) == -1) error_handling("bind error");
    if (listen(serv_sock, 5) == 1) error_handling("listen error");

    FD_ZERO(&_mainfds);
    FD_SET(serv_sock, &_mainfds);
    maxfd = serv_sock;
    while(1) {
        isWork = 0;
        checkfds = _mainfds;
        countProc = select(maxfd+1, &checkfds, (fd_set*)0, (fd_set*)0, NULL);
        if (countProc < 0) {
            if (errno == EINTR) continue;
            else {
                printf("return of select is -1, not EINTR.\terrno: %s\n", strerror(errno));
                exit(1);
            }
        } else if (countProc == 0) {
            continue;
        } else {
            if(FD_ISSET(serv_sock, &checkfds)) {
                tmp_sock = accept(serv_sock, (struct sockaddr *) &cli_addr, &cli_addr_size);
                if(tmp_sock == -1) {
                    if(errno != EAGAIN) error_handling("accept error");
                }
                else if(con_count == MAX_CONNECT) {
                    printf("full Room\n");
                }
                else {
                    for (int i = 0; i < MAX_CONNECT; i++) {
                        if(_client[i].socket == -1) {
                            _client[i].socket = tmp_sock;
                            fcntl(_client[i].socket, F_SETFL, O_NONBLOCK);
                            _client[i].bufferPtr = _client[i].msgBuffer;
                            printf("[new user %d enter the room.]\n", id);
                            con_count++;
                            id++;
                            break;
                        }
                    }
                    if(tmp_sock > maxfd) maxfd = tmp_sock;
                    FD_SET(tmp_sock, &_mainfds);
                }
            }

            //read message
            for (int i = 0; i < MAX_CONNECT; i++)
            {
                if (_client[i].socket != -1 && FD_ISSET(_client[i].socket, &checkfds))
                {
                    //int ret = readMsg(_client[i].socket, _client[i].msgBuffer, &_client[i].bufferPtr);
                    int ret = read(_client[i].socket, _client[i].bufferPtr, MAX_BUFFER_SIZE-(_client[i].bufferPtr-_client[i].msgBuffer));

                    int tmpSocket = _client[i].socket;
                    //disconnect socket
                    if(ret == 0 || (ret == -1 && errno != EAGAIN)) {
                        printf("[user %s leave the room]\n",_client[i].nickname);
                        if(_client[i].room_id != 0) {
                            stLeave_room_req leaveRoom;
                            leaveRoom.room_id = _client[i].room_id;
                            proc_leave_room(&leaveRoom, i);
                        }
                        close(_client[i].socket);
                        _client[i].socket = -1;
                        con_count--;
                        FD_CLR(tmpSocket, &_mainfds);
                        if(tmpSocket== maxfd) maxfd = get_curMaxfd();
                    } else if(ret > 0) {
                        _client[i].bufferPtr += ret;
#if 0                           // *******make Function               
                        printf("buffer: ");
                        for(char* ptr = _client[i].msgBuffer; ptr <= _client[i].bufferPtr; ptr++) {
                            if(*ptr != '\0') printf("%c", *ptr);
                            else printf(" ");
                        }
                        printf("\n");
#endif
                        while ((ret = PopCompleteMsgFromBuf(_client[i].msgBuffer, &_client[i].bufferPtr, subStr)) == 1) {
                            json_decode(subStr, &msg, i, &maxfd);
                        }
                    }
                }
            }
        }
    }
}

int get_curMaxfd() 
{
    int maxfd = 0;
    
    for (int i = 0; i < MAX_CONNECT; i++)
    {
        if(_client[i].socket > maxfd) maxfd = _client[i].socket;
    }
    return maxfd;
}

int json_decode(char* jsonStr, uniMsg* msg, int cliIdx, int *maxfd) 
{
    json_object *msgObj, *msgType;

    msgObj = json_tokener_parse(jsonStr);
    msgType = json_object_object_get(msgObj, "msgType");
    //printf("msgType: %d\n\n", json_object_get_int(msgType));  ////////////
    if (msgType != NULL) {
        switch (json_object_get_int(msgType))
        {
        case MSG_LOGIN:
            printf("login req\n");
            if(json_decode_login_req(msgObj, &msg->uniReq.login)) {
                proc_login(&msg->uniReq.login, cliIdx);
            }
            break;
        case MSG_CRT_ROOM:
            printf("crt_room req\n");
            if(json_decode_crt_room_req(msgObj, &msg->uniReq.crt_room)) {
                proc_crt_room(&msg->uniReq.crt_room, cliIdx);
            }
            break;
        case MSG_JOIN_ROOM:
            printf("join_room req\n");
            if(json_decode_join_room_req(msgObj, &msg->uniReq.join_room)) {
                proc_join_room(&msg->uniReq.join_room, cliIdx);
            }
            break;
        case MSG_ROOM_CFG:
            printf("config_room rpt\n");
            if(json_decode_config_room_rpt(msgObj, &msg->uniRpt.config)) {
                if(proc_config_room(msg, cliIdx)) {
                    int broadcastRst = broadcastMsg(jsonStr, _client[cliIdx].room_id-1, -1);
                    if(broadcastRst == -1) printf("broadcast option config fail.\n");
                }
            }
            break;
        case MSG_GAME_START:
            printf("game_start req\n");
            if(json_decode_game_start_req(msgObj, &msg->uniReq.game_start)) {
                proc_game_start(&msg->uniReq.game_start, cliIdx);
            }
            break;
        case MSG_GAME_CTRL:
            printf("game_ctrl rpt\n");
            if(json_decode_game_ctrl_rpt(msgObj, &msg->uniRpt.game_control)) {
                if(proc_game_control(msg)) {
                    int broadcastRst = broadcastMsg(jsonStr, _client[cliIdx].room_id-1, -1);
                    if(broadcastRst == -1) printf("broadcast game control fail.\n");
                }
            }
            break;
        case MSG_DEL_ROOM:
            printf("del_room req\n");
            if(json_decode_del_room_req(msgObj, &msg->uniReq.delete_room)) {
                proc_delete_room(&msg->uniReq.delete_room, cliIdx);
            }
            break;
        case MSG_LEAVE_ROOM:
            printf("leave_room req\n");
            if(json_decode_leave_room_req(msgObj, &msg->uniReq.leave_room)) {
                proc_leave_room(&msg->uniReq.leave_room, cliIdx);
            }
            break;
        case MSG_HANDOVER_ADMIN:
            printf("handover_admin rpt\n");
            if(json_decode_handover_admin_rpt(msgObj, &msg->uniRpt.handover_admin)) {
                if(proc_handover_admin(msg, cliIdx)) {
                    int broadcastRst = broadcastMsg(jsonStr, _client[cliIdx].room_id-1, -1);
                    if(broadcastRst == -1) printf("broadcast handover_admin fail.\n");
                }
            }
            break;
        case MSG_KICK_USER:
            printf("kick_user rpt\n");
            if(json_decode_kick_user_rpt(msgObj, &msg->uniRpt.kick_user)) {
                if(proc_kick_user(msg, cliIdx)) {
                    int broadcastRst = broadcastMsg(jsonStr, _client[cliIdx].room_id-1, -1);
                    if(broadcastRst == -1) printf("broadcast handover_admin fail.\n");
                }
            }
            break;
        case MSG_QRY_ROOM_LIST:
            printf("room_list req\n");
            if(json_decode_room_list_req(msgObj, &msg->uniReq.room_list)) {
                proc_room_list(&msg->uniReq.room_list, cliIdx);
            }
            break;
        case MSG_POSITION:
            if(json_decode_position_rpt(msgObj, &msg->uniRpt.position)) {
                if(proc_position(msg, cliIdx)) {
                    int broadcastRst = broadcastMsg(jsonStr, _client[cliIdx].room_id-1, cliIdx);
                    if(broadcastRst == -1) printf("broadcast position fail.\n");
                }
            }
            break;
        case MSG_CATCH:
            printf("catch rpt\n");
            if(json_decode_catch_rpt(msgObj, &msg->uniRpt.catchSig)) {
                int catchRet = proc_catch_runner(&msg->uniRpt.catchSig, cliIdx);
                if(catchRet == -1) printf("catch fail: proc_catch_runner return -1\n");
                else if(catchRet == 0) {
                    int broadcastRst = broadcastMsg(jsonStr, _client[cliIdx].room_id-1, -1);
                    if(broadcastRst == -1) printf("broadcast catch fail.\n");
                    msg->uniRsp.game_end.totalTime = calcTotalTime(_client[cliIdx].room_id-1);
                    json_encode_game_end_rsp(&msg->uniRsp.game_end, cliIdx);
                }
                else {
                    int broadcastRst = broadcastMsg(jsonStr, _client[cliIdx].room_id-1, -1);
                    if(broadcastRst == -1) printf("broadcast catch fail.\n");
                }
            }
            break;
        case MSG_BLOCK:
            printf("block rpt\n");
            if(json_decode_block_rpt(msgObj, &msg->uniRpt.blockSig)) {
                if(proc_block(&msg->uniRpt.blockSig, cliIdx)) {
                    int broadcastRst = broadcastMsg(jsonStr, _client[cliIdx].room_id-1, -1);
                    if(broadcastRst == -1) printf("broadcast block fail.\n");
                }
            }
            break;
        case MSG_GAME_END:
            printf("game_end rsp\n");
            if(json_decode_game_end_req(msgObj, &msg->uniReq.game_end)) {
                if(proc_game_end(&msg->uniReq.game_end, cliIdx)) {
                    msg->uniRsp.game_end.totalTime = calcTotalTime(_client[cliIdx].room_id-1);
                    json_encode_game_end_rsp(&msg->uniRsp.game_end, cliIdx);
                }
            }
            break;
        #if 0
        case MSG_CHAT:
            //printf("receive message from user %d. echo to other clients.\n",msg->header.id);
            for(int j = 0; j < MAX_CONNECT; j++) {
                if (cliIdx != j && _client[j].socket != -1) {
                    if(writeMsg(_client[j].socket, msg, 500) != 1) {
                        printf("echo error\n");
                    }
                }
            }
            break;
        case MSG_HELLO:
            printf("msgName HELLO\n");
            break;
        case MSG_BYE:
            //printf("[user %d leave the room]\n",msg->header.id);
            close(_client[cliIdx].socket);
            _client[cliIdx].socket = -1;
            con_count--;
            FD_CLR(tmpSocket, &_mainfds);
            if(tmpSocket == *maxfd) *maxfd = get_curMaxfd();
            break;
        case MAX_MSG_VALUE:
        #endif
        default:
            break;
        }
        json_object_put(msgObj);
    } else {
        printf("switch not run\n");
        return 0;
    }
    return 1;


    #if 0
    int tmpSocket = _client[cliIdx].socket;
    switch (msg->msgType)
    {
        case MSG_LOGIN:
            proc_login(&msg->body.uniReq.login, cliIdx);
            break;
        case MSG_CRT_ROOM:
            proc_crt_room(&msg->body.uniReq.crt_room, cliIdx);
            break;
        case MSG_JOIN_ROOM:
            proc_join_room(&msg->body.uniReq.join_room, cliIdx);
            break;
        case MSG_ROOM_CFG:
            proc_config_room(msg, cliIdx);
            break;
        case MSG_GAME_START:
            proc_game_start(&msg->body.uniReq.game_start, cliIdx);
            break;
        case MSG_GAME_CTRL:
            proc_game_control(msg);
            break;
        case MSG_DEL_ROOM:
            proc_delete_room(&msg->body.uniReq.delete_room, cliIdx);
            break;
        case MSG_LEAVE_ROOM:
            proc_leave_room(&msg->body.uniReq.leave_room, cliIdx);
            break;
        case MSG_HANDOVER_ADMIN:
            proc_handover_admin(msg, cliIdx);
            break;
        case MSG_KICK_USER:
            proc_kick_user(msg, cliIdx);
            break;
        case MSG_QRY_ROOM_LIST:
            proc_room_list(&msg->body.uniReq.room_list, cliIdx);
            break;
        case MSG_POSITION:
            proc_position(msg, cliIdx);
            break;
        case MSG_CATCH:
            proc_catch_runner(&msg->body.uniReq.catchSig, cliIdx);
            break;
        case MSG_CHAT:
            //printf("receive message from user %d. echo to other clients.\n",msg->header.id);
            for(int j = 0; j < MAX_CONNECT; j++) {
                if (cliIdx != j && _client[j].socket != -1) {
                    if(writeMsg(_client[j].socket, msg, 500) != 1) {
                        printf("echo error\n");
                    }
                }
            }
            break;
        case MSG_HELLO:
            printf("msgName HELLO\n");
            break;
        case MSG_BYE:
            //printf("[user %d leave the room]\n",msg->header.id);
            close(_client[cliIdx].socket);
            _client[cliIdx].socket = -1;
            con_count--;
            FD_CLR(tmpSocket, &_mainfds);
            if(tmpSocket == *maxfd) *maxfd = get_curMaxfd();
            break;
        case MAX_MSG_VALUE:
        default:
            printf("unknown message. msgName: %d\n", msg->header.msgType);
            break;
    }
    #endif
}