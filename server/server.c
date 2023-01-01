#include "myNP.h"



int get_curMaxfd();
void ServerHandler(message *msg, int cliIdx, int *maxfd);
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

    char buff[MSG_LENGTH];
    message msg;
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
                    int ret = readMsg(_client[i].socket, &msg);
                    int tmpSocket = _client[i].socket;
                    //disconnect socket
                    if(ret == -1) {
                        printf("[user %s leave the room]\n",_client[i].nickname);
                        close(_client[i].socket);
                        _client[i].socket = -1;
                        con_count--;
                        FD_CLR(tmpSocket, &_mainfds);
                        if(tmpSocket== maxfd) maxfd = get_curMaxfd();
                    }
                    else if (ret == 1) {
                        ServerHandler(&msg, i, &maxfd);
                        
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

void ServerHandler(message *msg, int cliIdx, int *maxfd) 
{
    int tmpSocket = _client[cliIdx].socket;
    switch (msg->header.msgType)
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
            proc_catch_runner(&msg->body.uniReq.catch, cliIdx);
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
}
