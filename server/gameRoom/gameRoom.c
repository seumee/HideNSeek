#include "networkFunc/myNP.h"

stGameRoom _gameRoom[MAX_ROOM_NUMBER] = {0,};

// return value : true(1) / false(0)
int IsRoomIdValid(int room_id) 
{
    if((room_id-1) < 0 || (room_id-1) >= MAX_ROOM_NUMBER) return 0;
    return 1;
}

// return value : true(1) / false(0)
int IsRoomUsed(int room_id) 
{
    if(IsRoomIdValid(room_id) == 0) return 0;
    if(_gameRoom[room_id-1].roomname[0] == '\0') return 0;
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

int checkUserInRoom(int room_id, int index) {
    int roomIdx = room_id - 1;
    stGameRoom *curRoom = &_gameRoom[roomIdx];
    if(curRoom->user[index] != -1) return 1;
    else return 0;
}

// return value : true(1) / false(0)
int IsTagger(int room_id, int cliIdx)
{
    int roomIdx = room_id - 1;
    stGameRoom *curRoom = &_gameRoom[roomIdx];
    for (int i = 0; i < MAX_ROOM_SIZE; i++)
    {
        if(curRoom->user[i] == cliIdx) {
            if(curRoom->role[i] == ROLE_TAGGER) return 1;
        }
    }
    return 0;
}

// return value : success(new admin index) / fail(0)
int findNewAdmin(int room_id) {
    int roomIdx = room_id - 1;
    stGameRoom *curRoom = &_gameRoom[roomIdx];
    for (int i = 0; i < curRoom->userCount; i++)
    {
        if(curRoom->user[i] != -1) return i;
    }
    return 0;
}

void setDefaultRoomOptions(int roomIdx)
{
    _gameRoom[roomIdx].options.numberOfTagger = 2;
    _gameRoom[roomIdx].options.gameSpeed = 100;
    _gameRoom[roomIdx].options.sight_disadvantage = 30;
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
                for(int i = 0; i < MAX_ROOM_SIZE; i++) _gameRoom[roomIdx].user[i] = EMPTY_SLOT;
                _gameRoom[roomIdx].user[0] = creator;
                // set room option as default;
                setDefaultRoomOptions(roomIdx);
                return roomIdx + 1;
            }
        }
    }
    return -1;
}

//return value: -1: fail to join)Running room/   -2: fail to join)full room    / Others: userCount
int joinRoom(int player, int roomId) 
{
    int roomIdx = roomId - 1;
    int i = 0;
    
    if(_gameRoom[roomIdx].roomState != ROOM_STS_WAIT) {
        return -1;
    } else if(_gameRoom[roomIdx].userCount >= MAX_ROOM_SIZE) {
        return -2;
    }
    for(i; i < MAX_ROOM_SIZE; i++) {
        if(_gameRoom[roomIdx].user[i] == -1) {
            _gameRoom[roomIdx].user[i] = player;
            _gameRoom[roomIdx].userCount++;
            break;
        }
    }
    return i;
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
    for(int i = 0; i < MAX_ROOM_SIZE; i++) _gameRoom[roomIdx].role[i] = ROLE_RUNNER;
    while(taggerCount != curRoom->options.numberOfTagger) {
        printf("pick tagger!\n");
        int *curSlot = &curRoom->role[rand()%curRoom->userCount];
        if(*curSlot != ROLE_TAGGER && *curSlot != EMPTY_SLOT) {
            printf("%d to ROLE_TAGGER\n", *curSlot);
            *curSlot = ROLE_TAGGER;
            taggerCount++;
        }
    }
    _gameRoom[roomIdx].blockCooldown = 0;
    _gameRoom[roomIdx].roomState = ROOM_STS_RUNNING;
    return 1;
}

int endGame(int roomId) {
    int roomIdx = roomId - 1;
    stGameRoom* curRoom = &_gameRoom[roomIdx];
    curRoom->roomState = ROOM_STS_WAIT;
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
    stGameRoom* curRoom = &_gameRoom[roomIdx];

    curRoom->roomState = ROOM_STS_IDLE;
    for(int i = 0; i < curRoom->userCount; i++) {
        _client[curRoom->user[i]].room_id = 0;
        curRoom->user[i] = 0;
    }
    curRoom->userCount = 0;
    curRoom->admin = 0;
    curRoom->roomname[0] = '\0';
    return 1;
}

// return value 1:success with admin not changed, 0:admin changed, -1:leave fail(or fail to change admin)
int leaveRoom(int roomId, int player)
{
    int roomIdx = roomId - 1;
    stGameRoom *curRoom = &_gameRoom[roomIdx];
    //only one user in room -> delete room
    if(curRoom->userCount == 1) {
        _client[player].room_id = 0;
        deleteRoom(roomId);
        /////delete room
        return 1;
    } else {    
        for (int i = 0; i < curRoom->userCount; i++)
        {
            if(player == curRoom->user[i]) {
                // set user slot as 0
                _client[curRoom->user[i]].room_id = 0;
                curRoom->user[i] = -1;
                curRoom->userCount--;
                // if leave user is admin, change room admin to new index 0 user.
                if(player == curRoom->admin) {
                    int newAdmin = findNewAdmin(roomId);
                    if(newAdmin == 0) return -1;
                    curRoom->admin = newAdmin;
                    return 0;
                }
                return i;
            }
        }
    }
    return -1;
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
            curRoom->user[i] = -1;
            break;
        }
    }
    curRoom->userCount--;
    return 1;
}

//return value -1:fail  0:no next page    1:more room(next page exist)
int roomList(int page, stRoomList *roomList)
{
    int curPage = page -1;
    int count = 0;
    for(int i = curPage*10; i < MAX_ROOM_NUMBER; i++) {
        if(_gameRoom[i].roomname[0] != '\0') {
            if(count == PAGE_SIZE) {    //PAGE_SIZE가 꽉 참. -> 다음 페이지 존재 여부 확인 후 0/1 return
                if(_gameRoom[i+1].roomname[0] != '\0') return 1;
                else return 0;
            }
            else{
                roomList->roomId_list[i] = i+1;
                roomList->userCount_list[i] = _gameRoom[i].userCount;
                strcpy(roomList->roomName_list[i], _gameRoom[i].roomname);
                count++;
            }
        } else if(i == (curPage * 10)) {    //PAGE_SIZE가 차기 전에 빈 방 발견 / 방이 찾아지지 않았음.
            return -1;
        } else {    //PAGE_SIZE가 차기 전에 빈 방 발견 / 방이 하나라도 존재.
            return 0;
        }
    }
    return -1;  // return문에 걸리지 않고 함수 종료 -> fail
}

//return value: survivor count  -1:fail(msg sender is not tagger)
int catchRunner(int roomId, int catcherId)
{
    int roomIdx = roomId - 1;
    if(IsTagger(roomId, catcherId) == 0) return -1;
    _gameRoom[roomIdx].survive--;
    return _gameRoom[roomIdx].survive;
}

//return value: 1:success   -1:fail(block is in cooldown)
int blocking(int roomId) {
    int roomIdx = roomId - 1;
    //if(_gameRoom[roomIdx].blockCooldown)
    return 1;
}