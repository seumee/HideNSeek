#include "myNP.h"

stGameRoom _gameRoom[MAX_ROOM_NUMBER];

// return value : true(1) / false(0)
int IsRoomIdValid(int room_id) 
{
    if((room_id-1) < 0 || (room_id-1) >= MAX_ROOM_NUMBER) {
        return 0;
    }
    return 1;
}

// return value : true(1) / false(0)
int IsRoomUsed(int room_id) 
{
    if(IsRoomIdValid(room_id) == 0) return 0;

    if(_gameRoom[room_id-1].roomname[0] == '\0') {
        return 0;
    }
    return 1;
}

// return value : true(index in gameRoom) / false(-1)
int checkNicknameValid(int room_id, char *name)
{
    int roomIdx = room_id - 1;
    stGameRoom *curRoom = &_gameRoom[roomIdx];
    for (int i = 0; i < curRoom->userCount; i++)
    {
        if(strcmp(_client[curRoom->user[i]].nickname, name) == 0) return i;
    }
    return -1;
}

// return value : true(1) / false(0)
int IsTagger(int room_id, int cliIdx)
{
    int roomIdx = room_id - 1;
    stGameRoom *curRoom = &_gameRoom[roomIdx];
    for (int i = 0; i < curRoom->userCount; i++)
    {
        if(curRoom->user[i] == cliIdx) {
            if(curRoom->role[i] == ROLE_TAGGER) return 1;
        }
    }
    return 0;
}

void trimUser(int roomIdx, int trimIdx) 
{
    stGameRoom *curRoom = &_gameRoom[roomIdx];
    for(int i = trimIdx; i < MAX_ROOM_SIZE-1; i++) {
        curRoom->user[i] = curRoom->user[i+1];
    }
    curRoom->user[MAX_ROOM_SIZE-1] = 0;
}

void setDefaultRoomOptions(int roomIdx)
{
    _gameRoom[roomIdx].options.numberOfTagger = 2;
    _gameRoom[roomIdx].options.gameSpeed = 100;
    _gameRoom[roomIdx].options.sight_disadvantage = 80;
    _gameRoom[roomIdx].options.taggerSpeed = 120;
    _gameRoom[roomIdx].options.catch_cooldown = 10;
    _gameRoom[roomIdx].options.block_cooldown = 20;
}

int calcTotalTime(int roomIdx)
{
    return (int)(time(NULL) - _gameRoom[roomIdx].gametime);
}

/*-----------------------------------------------------------*/

// return value: roomId(-1: fail)
int createRoom(int creator, char *roomname)
{
    // find empty room
    int roomIdx;
    for (int i = 0; i < MAX_ROOM_NUMBER; i++)
    {
        if (_gameRoom[i].roomname[0] == '\0') {
            if (_gameRoom[i].roomState == ROOM_STS_IDLE) {
                _gameRoom[i].roomState = ROOM_STS_WAIT;
                roomIdx = i;
                // set room info
                strcpy(_gameRoom[roomIdx].roomname, roomname);
                _gameRoom[roomIdx].admin = creator;
                _gameRoom[roomIdx].userCount = 1;
                _gameRoom[roomIdx].user[0] = creator;
                // set room option as default;
                setDefaultRoomOptions(roomIdx);
                return roomIdx + 1;
                break;
            }
        }
    }
    return -1;
}

//return value: -1: fail to join / Others: userCount
int joinRoom(int player, int roomId) 
{
    int roomIdx = roomId - 1;
    if (_gameRoom[roomIdx].roomState == ROOM_STS_WAIT) {
        _gameRoom[roomIdx].user[_gameRoom[roomIdx].userCount] = player;
        return _gameRoom[roomIdx].userCount++;
    }
    return -1;
}

int configRoom(int roomId, stGame_option *options)
{
    int roomIdx = roomId - 1;
    memcpy(&_gameRoom[roomIdx].options, options, sizeof(stGame_option));
    return 1;
}

int startGame(int roomId)
{
    int roomIdx = roomId - 1;
    stGameRoom *curRoom = &_gameRoom[roomIdx];
    int randRst;
    int taggerCount = 0;

    //set rand seed
    srand(time(NULL));
    //set game start time
    curRoom->gametime = time(NULL);

    //set survivor count
    curRoom->survive = curRoom->userCount - curRoom->options.numberOfTagger;

    //set role
    while(taggerCount != curRoom->options.numberOfTagger) {
        randRst = rand()%curRoom->userCount;
        if(curRoom->role[randRst] != ROLE_TAGGER) {
            curRoom->role[randRst] = ROLE_TAGGER;
            taggerCount++;
        }
    }

    _gameRoom[roomIdx].roomState = ROOM_STS_RUNNING;
    return 1;
}

int gameControl(int roomId, int request)
{
    int roomIdx = roomId;
    _gameRoom[roomIdx].roomState = request;
    return 1;
}

int deleteRoom(int roomId) 
{
    int roomIdx = roomId - 1;
    _gameRoom[roomIdx].roomname[0] = '\0';
    return 1;
}

// return value 1:success with admin not changed, 0:admin changed.
int leaveRoom(int roomId, int player)
{
    int roomIdx = roomId - 1;
    stGameRoom *curRoom = &_gameRoom[roomIdx];
    for (int i = 0; i < curRoom->userCount; i++)
    {
        if(player == curRoom->user[i]) {
            // set user slot as 0, trim
            curRoom->user[i] = 0;
            trimUser(roomIdx, i);
            // if leave user is admin, change room admin to new index 0 user.
            if(player == curRoom->admin) {
                curRoom->admin = curRoom->user[0];
                return 0;
            }
        }
    }
    return 1;
}

int handoverAdmin(int roomId, int newAdmin)
{
    int roomIdx = roomId - 1;
    _gameRoom[roomIdx].admin = newAdmin;
    return 1;
}

int kickUser(int roomId, int kickTarget)
{
    int roomIdx = roomId - 1;
    stGameRoom *curRoom = &_gameRoom[roomIdx];
    for (int i = 0; i < curRoom->userCount; i++)
    {
        if(kickTarget == curRoom->user[i]) {
            // set user slot as 0, trim
            curRoom->user[i] = 0;
            trimUser(roomIdx, i);
        }
    }
    return 1;
}

//return value -1:fail  0:no next page    1:more room(next page exist)
int roomList(int page, stRoomList *roomList)
{
    int skip = page * 10;
    int count = 0;
    for(int i = 0; i < MAX_ROOM_NUMBER; i++) {
        if(_gameRoom[i].roomname[0] != '\0') {
            if(count == PAGE_SIZE) return 0;
            if(skip > 0) {
                skip--;
            } else{
            roomList->roomId_list[i] = i+1;
            strcpy(roomList->roomName_list[i], _gameRoom[i].roomname);
            count++;
            }
        }
    }
    return 1;
}

//return value: survivor count  -1:fail(msg sender is not tagger)
int catchRunner(int roomId, int catcherId)
{
    int roomIdx = roomId - 1;
    if(IsTagger(roomId, catcherId) == 0) return -1;
    else return --(_gameRoom[roomIdx].survive);
}