#ifndef MYROOM_H
#define MYROOM_H
#include "messageHandler/myMessage.h"
#include <time.h>

/*
enum colorIndex {
    COLOR_BLACK = 0,
    COLOR_BLUE,
    COLOR_BROWN,
    COLOR_GREEN,
    COLOR_ORANGE,
    COLOR_PINK,
    COLOR_RED,
    COLOR_YELLOW,
    COLOR_MAX_COUNT
};

typedef struct color
{
    int colors[COLOR_MAX_COUNT];
};
*/

enum roomState {
    ROOM_STS_IDLE = 0,
    ROOM_STS_WAIT,
    ROOM_STS_RUNNING,
    ROOM_STS_PAUSE,
    ROOM_STS_RESULT,
    MAX_ROOM_STS
};

#define ROLE_RUNNER 0
#define ROLE_TAGGER 1

#define EMPTY_SLOT -1

typedef struct {
    char roomname[NICKNAME_SIZE]; // NULL string : not created
    int admin;  // creater client_id of room
    int userCount; // current count of users
    int user[MAX_ROOM_SIZE];
    int role[MAX_ROOM_SIZE]; // role of users
    int roomState;
    int survive;
    time_t blockCooldown;
    time_t gametime;
    stGame_option options;
} stGameRoom;

int IsRoomIdValid(int room_id);
int IsRoomUsed(int room_id);
int IsTagger(int room_id, int cliIdx);
int findNewAdmin(int room_id);
void setDefaultRoomOptions(int roomIdx);
int checkNicknameValid(int room_id, char *name);
int checkUserInRoom(int room_id, int index);
int calcTotalTime(int roomIdx);

int createRoom(int creator, char *roomname);
int joinRoom(int player, int roomId);
int configRoom(int roomId, stGame_option *options);
int startGame(int roomId);
int endGame(int roomId);
int gameControl(int roomId, int request);
int deleteRoom(int roomId);
int leaveRoom(int roomId, int player);
int handoverAdmin(int roomId, int newAdmin);
int kickUser(int roomId, int kickTarget);
int roomList(int page, stRoomList *roomList);
int catchRunner(int roomId, int catcherId);
int blocking(int roomId);

extern stGameRoom _gameRoom[MAX_ROOM_NUMBER];

#endif