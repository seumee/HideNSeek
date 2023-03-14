#include "networkFunc/myNP.h"

//1: success.   0:fail
int moveToRoom(int cliIdx, int room_id)
{
    int roomIdx = room_id - 1;
    if(_gameRoom[roomIdx].roomState == ROOM_STS_WAIT) {
        _client[cliIdx].room_id = roomIdx+1;
        return 1;
    } else {
        return 0;
    }
}

void proc_login(stLogin_req *login, int cliIdx) 
{
    uniMsg msg;
    //login
    strcpy(_client[cliIdx].nickname, login->nickname);
    _client[cliIdx].room_id = 0;
    msg.uniRsp.login.result = RSP_VALUE_SUCC;
    json_encode_login_rsp(&msg.uniRsp.login, cliIdx);
    
    //printf("client's nickname: %s\t\t roomId: %d\n", _client[cliIdx].nickname, _client[cliIdx].room_id);
}


void proc_crt_room(stCrt_room_req *crt_room, int cliIdx) 
{
    uniMsg msg;
    //create room
    int roomId = createRoom(cliIdx, crt_room->roomname);
    //set body
    if(roomId == -1) {
        msg.uniRsp.crt_room.result = RSP_VALUE_FAIL;
        msg.uniRsp.crt_room.room_id = 0;
    } else {
        msg.uniRsp.crt_room.result = RSP_VALUE_SUCC;
        msg.uniRsp.crt_room.room_id = roomId;
        if(!moveToRoom(cliIdx, roomId)) {
            printf("moveToRoom failed.\n");
        }

    }
    json_encode_crt_room_rsp(&msg.uniRsp.crt_room, cliIdx);
}

void proc_join_room(stJoin_room_req *join_room, int cliIdx) 
{
    uniMsg msg;
    int index;
    int clientRoomCheck;
    int findAdminRes;
    stGameRoom *curRoom = &_gameRoom[(join_room->room_id)-1];
    //check is client joined room
    if(_client[cliIdx].room_id != 0) {
        printf("client already in room\n");
        clientRoomCheck = -1;
    }
    //check room_id valid
    if(IsRoomUsed(join_room->room_id) == 0) {
        index = -1;
    } else{
        //join room
        index = joinRoom(cliIdx, join_room->room_id);
        if(!moveToRoom(cliIdx, join_room->room_id)) {
            printf("moveToRoom failed. not existing room.\n");
        }
    }
    findAdminRes = findAdminColorIdxByCliIdx(cliIdx);

    //set body
    if(index == -1) {
        msg.uniRsp.join_room.result = RSP_VALUE_RUNNING_ROOM;
    } else if(index == -2) {
        msg.uniRsp.join_room.result = RSP_VALUE_FULL_ROOM;
    } else if (clientRoomCheck == -1 || findAdminRes == -1) {
        msg.uniRsp.join_room.result = RSP_VALUE_FAIL;
    } else {
        stGame_option *options = &curRoom->options;

        msg.uniRsp.join_room.result = curRoom->userCount;
        msg.uniRsp.join_room.colorIdx = index;
        msg.uniRsp.join_room.adminColorIdx = findAdminRes;

        msg.uniRsp.join_room.options.numberOfTagger = options->numberOfTagger;
        msg.uniRsp.join_room.options.gameSpeed = options->gameSpeed;
        msg.uniRsp.join_room.options.sight_disadvantage = options->sight_disadvantage;
        msg.uniRsp.join_room.options.taggerSpeed = options->taggerSpeed;
        msg.uniRsp.join_room.options.catch_cooldown = options->catch_cooldown;
        msg.uniRsp.join_room.options.block_cooldown = options->block_cooldown;
    }
    //send result
    json_encode_join_room_rsp(&msg.uniRsp.join_room, cliIdx);
}

//1:success,    0:fail
int proc_config_room(uniMsg *msg, int cliIdx) 
{
    //extract body from msg
    stConfig_room_rpt *config = &msg->uniRpt.config;
    int roomIdx = config->room_id-1;

    //check valid
    if(IsRoomUsed(config->room_id) == 0) {
        printf("rpt_config_room fail: room_id invalid\n");
        return 0;
    }
    if(cliIdx != _gameRoom[roomIdx].admin) {
        printf("rpt_config_room fail: configuer is not admin\n");
        return 0;
    }

    //config options
    configRoom(config->room_id, &config->options);
    return 1;
}

void proc_game_start(stGame_start_req *game_start, int cliIdx)
{
    uniMsg msg;
    int roomIdx = game_start->room_id-1;
    stGameRoom *curRoom = &_gameRoom[roomIdx];

    //check valid
    if(IsRoomUsed(game_start->room_id) == 0) {
        printf("rsp_game_start fail: room_id invalid\n");
        msg.uniRsp.game_start.result= RSP_VALUE_FAIL;
    }
    else if(cliIdx != curRoom->admin) {
        printf("rsp_game_start fail: configuer is not admin\n");
        msg.uniRsp.game_start.result = RSP_VALUE_FAIL;
    }
    else if(curRoom->roomState != ROOM_STS_WAIT) {
        printf("rsp_game_start fail: this room is not in status \"WAIT\"\n");
        msg.uniRsp.game_start.result = RSP_VALUE_FAIL;
    }
    else if(curRoom->options.numberOfTagger >= curRoom->userCount) {
        printf("rsp_game_start fail: game option \"numberOfTagger\" is bigger than userCount.\n");
        msg.uniRsp.game_start.result = RSP_VALUE_FAIL;
    }
    else {
        //start game
        int isStarted = startGame(game_start->room_id);

        if (isStarted == -1) printf("startGame fail\n");
        else {
            //set body
            msg.uniRsp.game_start.result = RSP_VALUE_SUCC;
            for(int i = 0; i < curRoom->userCount; i++) {
                msg.uniRsp.game_start.role[i] = curRoom->role[i];
            }
        }
    }
    //send result
    json_encode_game_start_rsp(&msg.uniRsp.game_start, cliIdx);
}

int proc_game_control(uniMsg *msg)
{
    stGame_ctrl_rpt *game_ctrl = &msg->uniRpt.game_control;
    int roomIdx = game_ctrl->room_id-1;
    stGameRoom *curRoom = &_gameRoom[roomIdx];

    //check valid
    if(IsRoomUsed(game_ctrl->room_id) == 0) {
        printf("rpt_game_control fail: room_id invalid\n");
        return 0;
    }
    else if(curRoom->roomState != ROOM_STS_RUNNING && curRoom->roomState != ROOM_STS_PAUSE) {
        printf("rsp_game_start fail: this room is not in status \"RUNNING\" or \"PAUSE\"\n");
        return 0;
    }
    //control game
    gameControl(game_ctrl->room_id, game_ctrl->status);
    return 1;
}

/////////// change _client info about non-admin users
void proc_delete_room(stDel_room_req *del_room, int cliIdx)
{
    stDel_room_rsp rspMsg;
    int isDelete;
    //check valid
    if(IsRoomUsed(del_room->room_id) == 0) {
        isDelete = -1;
        printf("proc_delete_room fail: room_id invalid\n");
        return;
    } else {
        //delete room
        isDelete = deleteRoom(del_room->room_id);
    }
    //set body
    if (isDelete == -1) rspMsg.result = RSP_VALUE_FAIL;
    else rspMsg.result = RSP_VALUE_SUCC;
    //send result
    json_encode_del_room_rsp(&rspMsg, cliIdx);
}

void proc_leave_room(stLeave_room_req *leave_room, int cliIdx) {
    printf("proc_leave_room proc.\n");
    stLeave_room_rsp rspMsg;
    int whoLeave;
    //check valid
    if(IsRoomUsed(leave_room->room_id) == 0) {
        whoLeave = -1;
        printf("proc_leave_room fail: room_id invalid\n");
        return;
    } else {
        //leave room
        whoLeave = leaveRoom(leave_room->room_id, cliIdx);
    }
    //set body
    //fail  result:RSP_VALUE_FAIL
    if (whoLeave == -1) {
        rspMsg.whoLeave = RSP_VALUE_FAIL;
    }
    //success and admin change  result:who leave   admin:new admin's colorIdx
    else if(whoLeave == 0) {
        rspMsg.whoLeave = whoLeave;
        rspMsg.adminColorIdx = _gameRoom[leave_room->room_id-1].admin;
    }
    //success and admin not change  result:who leave    admin:-1
    else {
        rspMsg.whoLeave = whoLeave;
        rspMsg.adminColorIdx = -1;
    }
    //send result       //jsonHandle 추가 후, broadcast 하도록 수정 필요.
    json_encode_leave_room_rsp(&rspMsg, cliIdx, leave_room->room_id);
}

int proc_handover_admin(uniMsg *msg, int cliIdx)
{
    //extract body from msg
    stHandover_admin_rpt *handover = &msg->uniRpt.handover_admin;

    int roomIdx = handover->room_id-1;
    //check valid
    if(IsRoomUsed(handover->room_id) == 0) {
        printf("handover_admin fail: room_id invalid\n");
        return 0;
    }
    if(cliIdx != _gameRoom[roomIdx].admin) {
        printf("handover_admin fail: msg not from admin\n");
        return 0;
    }

    //handover admin        //newAdmin을 String으로 관리하도록 수정 필요.
    int newAdmin = findUserByName(roomIdx, handover->newAdmin);
    if(newAdmin == -1) {
        printf("handover_admin->findUserByName fail. There is no User match with nickname.\n");
        return 0;
    } else {
        if(handoverAdmin(handover->room_id, newAdmin) == 0) {
            printf("handoverAdmin fail.\n");
            return 0;
        }
    }
    return 1;
}

int proc_kick_user(uniMsg *msg, int cliIdx)
{
    //extract body from msg
    stKick_user_rpt *kick = &msg->uniRpt.kick_user;

    int roomIdx = kick->room_id-1;
    int isKicked;
    //check valid
    if(IsRoomUsed(kick->room_id) == 0) {
        printf("kick_user fail: room_id invalid\n");
        return 0;
    }
    if(cliIdx != _gameRoom[roomIdx].admin) {
        printf("kick_user fail: msg not from admin\n");
        return 0;
    }

    //kick user
    int kickTarget = findUserByName(roomIdx, kick->kickTarget);
    if(kickTarget == -1) {
        printf("handover_admin->findUserByName fail. There is no User match with nickname.\n");
        return 0;
    } else {
        if(kickUser(kick->room_id, kickTarget) == 0) {
            printf("kickUser fail.\n");
            return 0;
        }
    }
    return 1;
}

void proc_room_list(stRoom_list_req *room_list, int cliIdx)
{
    stRoom_list_rsp rspMsg = {0,};
    stRoomList retRoomList = {0,};
    // set room list
    int isNext = roomList(room_list->page, &retRoomList);
    rspMsg.page = room_list->page;
    // set body
    if(isNext == -1) {
        rspMsg.isNext = RSP_QRY_FAIL;
    } else {
        if(isNext == 1) rspMsg.isNext = RSP_QRY_ISNEXT;
        else rspMsg.isNext = RSP_QRY_NONEXT;
        memcpy(&rspMsg.room_list, &retRoomList, sizeof(stRoomList));
    }
    // send result
    json_encode_room_list_rsp(&rspMsg, cliIdx);
}

int proc_position(uniMsg *msg, int cliIdx)
{
    //extract body from msg
    stPosition_rpt *position = &msg->uniRpt.position;
    int roomIdx = position->room_id - 1;
    //check valid
    if(IsRoomUsed(position->room_id) == 0) {
        printf("proc_position fail: room_id invalid\n");
        return 0;
    }
    if(findUserByName(roomIdx, position->nickname) == -1) {
        printf("proc_position->findUserByName fail. There is no User match with nickname.\n");
        return 0;
    }
    return 1;
}

//return > 0: game running & catch,   0: game end,    -1: fail
int proc_catch_runner(stCatch_rpt *catch, int cliIdx)
{
    uniMsg rspMsg;
    //check valid
    if(IsRoomUsed(catch->room_id) == 0) {
        printf("catch fail: room_id invalid\n");
        return -1;
    }
    if(checkUserInRoom(catch->room_id, catch->catchWhom) == 0) {
        printf("catch fail: target index invalid\n");
        return -1;
    }
    if(checkUserInRoom(catch->room_id, catch->tagger) == 0) {
        printf("catch fail: tagger index invalid\n");
        return -1;
    }
    //catch runner & return
    printf("return catchRunner.\n");
    return catchRunner(catch->room_id, cliIdx);
}


int proc_block(stBlock_rpt *block, int cliIdx) {
    uniMsg rspMsg;
    int roomIdx = block->room_id - 1;
    stGameRoom *curRoom = &_gameRoom[roomIdx];
    //check valid
    if(IsRoomUsed(block->room_id) == 0) {
        printf("block fail: room_id invalid\n");
        return 0;
    }
    if(checkUserInRoom(block->room_id, block->tagger) == 0) {
        printf("block fail: tagger index invalid\n");
        return 0;
    }
    if(time(NULL) - curRoom->blockCooldown >= curRoom->options.block_cooldown) curRoom->blockCooldown = time(NULL);
    else return 0;
    return 1;
}

//1:success,    0:fail
int proc_game_end(stGame_end_req *game_end, int cliIdx) 
{
    //check valid
    if(IsRoomUsed(game_end->room_id) == 0) {
        printf("rpt_config_room fail: room_id invalid\n");
        //return 0;
    }

    //config options
    endGame(game_end->room_id);
    return 1;
}