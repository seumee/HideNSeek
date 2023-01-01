#ifndef MYMESSAGE_H
#define MYMESSAGE_H

#define NICKNAME_SIZE 128
#define MSG_LENGTH 4096
#define MSG_ARR_SIZE 512
#define SERVER_ID 0
#define OPTION_NUM 6
#define PAGE_SIZE 10

typedef struct {
    int roomId_list[PAGE_SIZE];
    char roomName_list[PAGE_SIZE][NICKNAME_SIZE];
} stRoomList;

typedef struct {
    int numberOfTagger;
    int gameSpeed;
    int sight_disadvantage;
    int taggerSpeed;
    int catch_cooldown;
    int block_cooldown;
} stGame_option;

enum msgName_values{
    MSG_LOGIN = 0,
    MSG_CRT_ROOM,
    MSG_JOIN_ROOM,
    MSG_ROOM_CFG,
    MSG_GAME_START,
    MSG_GAME_CTRL,
    MSG_DEL_ROOM,
    MSG_LEAVE_ROOM,
    MSG_HANDOVER_ADMIN,
    MSG_KICK_USER,
    MSG_QRY_ROOM_LIST,
    MSG_POSITION,
    MSG_CATCH,
    MSG_GAME_END,   //server send only
    MSG_BLOCK,
    MSG_CHAT,
    MSG_HELLO,
    MSG_BYE,
    MAX_MSG_VALUE
};

enum gameState_values{ 
    STATUS_PAUSE = 0,
    STATUS_RESUME
};

enum bodyType_values{ 
    BODY_INT = 0,
    BODY_INT_ARRAY,
    BODY_STRING
};

#define RSP_VALUE_FAIL -1
#define RSP_VALUE_SUCC 0

#define RSP_QRY_FAIL -1
#define RSP_QRY_NONEXT 0
#define RSP_QRY_ISNEXT 1

struct header {
    int bodyLen;
    int msgType;
} typedef stHeader;

typedef struct {
    char nickname[NICKNAME_SIZE];
} stLogin_req;

typedef struct {
    int result;
} stLogin_rsp;

typedef struct {
    char roomname[NICKNAME_SIZE];
} stCrt_room_req;

typedef struct {
    int result;
    int room_id;
} stCrt_room_rsp;

typedef struct {
    int room_id;
} stJoin_room_req;

typedef struct {
    int result;
    int colorIdx;
} stJoin_room_rsp;

typedef struct {
    int room_id;
    stGame_option options;
} stConfig_room_rpt;

typedef struct {
    int room_id;
} stGame_start_req;

typedef struct {
    int role;
} stGame_start_rsp;

typedef struct {
    int room_id;
    int status;
} stGame_ctrl_rpt;

typedef struct {
    int room_id;
} stDel_room_req;

typedef struct {
    int result;
} stDel_room_rsp;

typedef struct {
    int room_id;
} stLeave_room_req;

typedef struct {
    int result;
    char admin[NICKNAME_SIZE];
} stLeave_room_rsp;

typedef struct {
    int room_id;
    char newAdmin[NICKNAME_SIZE];
} stHandover_admin_rpt;

typedef struct {
    int room_id;
    char kickTarget[NICKNAME_SIZE];
} stKick_user_rpt;

typedef struct {
    int page;
} stRoom_list_req;

typedef struct {
    int isNext;
    stRoomList room_list;
} stRoom_list_rsp;

typedef struct {
    int room_id;
    char nickname[NICKNAME_SIZE];
    int isAlive;
    int x;
    int y;
} stPosition_rpt;

typedef struct {
    int room_id;
    char catchWhom[NICKNAME_SIZE];
} stCatch_req;

typedef struct {
    char catchWhom[NICKNAME_SIZE];
} stCatch_rsp;

typedef struct {
    int totalTime;
} stGame_end_rsp;

union body {
    union {
        stLogin_req login;
        stCrt_room_req crt_room;
        stJoin_room_req join_room;
        stDel_room_req delete_room;
        stLeave_room_req leave_room;
        stGame_start_req game_start;
        stRoom_list_req room_list;
        stCatch_req catch;
    } uniReq;
    union {
        stLogin_rsp login;
        stCrt_room_rsp crt_room;
        stJoin_room_rsp join_room;
        stDel_room_rsp delete_room;
        stGame_start_rsp game_start;
        stLeave_room_rsp leave_room;
        stRoom_list_rsp room_list;
        stCatch_rsp catch;
        stGame_end_rsp game_end;
    } uniRsp;
    union {
        stConfig_room_rpt config;
        stGame_ctrl_rpt game_control;
        stHandover_admin_rpt handover_admin;
        stKick_user_rpt kick_user;
        stPosition_rpt position;
    } uniRpt;
} typedef uniBody;

struct msg {
    stHeader header;
    uniBody body;
} typedef message;

void moveToRoom(int cliIdx, int room_id);
void proc_login(stLogin_req *login, int cliIdx);
void proc_crt_room(stCrt_room_req *crt_room, int cliIdx);
void proc_join_room(stJoin_room_req *join_room, int cliIdx);
void proc_config_room(message *msg, int cliIdx);
void proc_game_start(stGame_start_req *game_start, int cliIdx);
void proc_game_control(message *msg);
void proc_delete_room(stDel_room_req *del_room, int cliIdx);
void proc_leave_room(stLeave_room_req *leave_room, int cliIdx);
void proc_handover_admin(message *msg, int cliIdx);
void proc_kick_user(message *msg, int cliIdx);
void proc_room_list(stRoom_list_req *room_list, int cliIdx);
void proc_position(message *msg, int cliIdx);
void proc_catch_runner(stCatch_req *catch, int cliIdx);

#endif
