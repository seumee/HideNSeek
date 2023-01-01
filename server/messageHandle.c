#include "myNP.h"

void moveToRoom(int cliIdx, int room_id)
{
    int roomIdx = room_id - 1;
    _client[cliIdx].room_id = roomIdx;
}

void proc_login(stLogin_req *login, int cliIdx) 
{
    message rspMsg;
    //login
    strcpy(_client[cliIdx].nickname, login->nickname);
    _client[cliIdx].room_id = 0;

    //set body
    rspMsg.body.uniRsp.login.result = RSP_VALUE_SUCC;
    //set header
    setHeader(&rspMsg.header, MSG_LOGIN, sizeof(stLogin_rsp));
    int sendRet = writeMsg(_client[cliIdx].socket, &rspMsg, 500);
    if(sendRet == -1) printf("rsp_login fail\n");
}

void proc_crt_room(stCrt_room_req *crt_room, int cliIdx) 
{
    message rspMsg;
    //create room
    int roomId = createRoom(cliIdx, crt_room->roomname);
    //set body
    if(roomId == -1) {
        rspMsg.body.uniRsp.crt_room.result = RSP_VALUE_FAIL;
        rspMsg.body.uniRsp.crt_room.room_id = 0;
    } else {
        rspMsg.body.uniRsp.crt_room.result = RSP_VALUE_SUCC;
        rspMsg.body.uniRsp.crt_room.room_id = roomId;
        moveToRoom(cliIdx, roomId);
    }
    //set header
    setHeader(&rspMsg.header, MSG_CRT_ROOM, sizeof(stCrt_room_rsp));
    //send result 
    int sendRet = writeMsg(_client[cliIdx].socket, &rspMsg, 500);
    if(sendRet == -1) printf("rsp_crt_room fail\n");
}

void proc_join_room(stJoin_room_req *join_room, int cliIdx) 
{
    message rspMsg;
    int index;
    int clientRoomCheck;
    //check is client joined room
    if(_client[cliIdx].room_id != 0) {
        printf("client already in room\n");
        //set header
        setHeader(&rspMsg.header, MSG_JOIN_ROOM, sizeof(stJoin_room_rsp));
        clientRoomCheck = -1;
    }
    //check room_id valid
    if(IsRoomUsed(join_room->room_id) == 0) {
        index = -1;
    } else{
        //join room
        index = joinRoom(cliIdx, join_room->room_id);
        moveToRoom(cliIdx, join_room->room_id);
    }

    //set body
    if (index == -1 || clientRoomCheck == -1) {
        rspMsg.body.uniRsp.join_room.result = RSP_VALUE_FAIL;
        rspMsg.body.uniRsp.join_room.colorIdx = -1;
    }
    else {
        rspMsg.body.uniRsp.join_room.result = RSP_VALUE_SUCC;
        rspMsg.body.uniRsp.join_room.colorIdx = index;
    }
    //set header
    setHeader(&rspMsg.header, MSG_JOIN_ROOM, sizeof(stJoin_room_rsp));
    //send result
    int sendRet = writeMsg(_client[cliIdx].socket, &rspMsg, 500);
    if(sendRet == -1) printf("rsp_join_room fail\n");
}

void proc_config_room(message *msg, int cliIdx) 
{
    //extract body from msg
    stConfig_room_rpt *config = &msg->body.uniRpt.config;
    int roomIdx = config->room_id-1;

    //check valid
    if(IsRoomUsed(config->room_id) == 0) {
        printf("rpt_config_room fail: room_id invalid\n");
        return;
    }
    if(cliIdx != _gameRoom[roomIdx].admin) {
        printf("rpt_config_room fail: configuer is not admin\n");
        return;
    }

    //config options
    configRoom(config->room_id, &config->options);

    //broadcast option config message
    int broadcastRst = broadcastMsg(msg, roomIdx, cliIdx);
    if(broadcastRst == -1) printf("broadcast option config fail.\n");
}

void proc_game_start(stGame_start_req *game_start, int cliIdx)
{
    message rspMsg;
    int roomIdx = game_start->room_id-1;
    stGameRoom *curRoom = &_gameRoom[roomIdx];

    //check valid
    if(IsRoomUsed(game_start->room_id) == 0) {
        printf("rsp_game_start fail: room_id invalid\n");
        return;
    }
    if(cliIdx != curRoom->admin) {
        printf("rsp_game_start fail: configuer is not admin\n");
        return;
    }

    //start game
    int isStarted = startGame(game_start->room_id);

    //set header
    setHeader(&rspMsg.header, MSG_GAME_START, sizeof(stGame_start_rsp));

    if (isStarted == -1) printf("startGame fail\n");
    else {
        //set body
        for(int i = 0; i < curRoom->userCount; i++) {
            rspMsg.body.uniRsp.game_start.role = curRoom->role[i];
            //send result
            int sendRet = writeMsg(_client[curRoom->user[i]].socket, &rspMsg, 500);
            if(sendRet == -1) printf("rsp_game_start fail in cliIdx %d\n", curRoom->user[i]);
        }
    }
}

void proc_game_control(message *msg)
{
    //extract body from msg
    stGame_ctrl_rpt *game_ctrl = &msg->body.uniRpt.game_control;
    int roomIdx = game_ctrl->room_id-1;

    //check valid
    if(IsRoomUsed(game_ctrl->room_id) == 0) {
        printf("rpt_game_control fail: room_id invalid\n");
        return;
    }

    //control game
    gameControl(game_ctrl->room_id, game_ctrl->status);

    //broadcast control message, include messege sender
    int broadcastRst = broadcastMsg(msg, roomIdx, -1);
    if(broadcastRst == -1) printf("broadcast game control fail.\n");
}


/////////// change _client info about non-admin users
void proc_delete_room(stDel_room_req *del_room, int cliIdx)
{
    message rspMsg;
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
    if (isDelete == -1) rspMsg.body.uniRsp.delete_room.result = RSP_VALUE_FAIL;
    else rspMsg.body.uniRsp.delete_room.result = RSP_VALUE_SUCC;
    //set header
    setHeader(&rspMsg.header, MSG_DEL_ROOM, sizeof(stDel_room_rsp));
    //send result
    int sendRet = writeMsg(_client[cliIdx].socket, &rspMsg, 500);
    if(sendRet == -1) printf("rsp_delete_room fail\n");
}

void proc_leave_room(stLeave_room_req *leave_room, int cliIdx) {
    message rspMsg;
    int isLeaved;
    //check valid
    if(IsRoomUsed(leave_room->room_id) == 0) {
        isLeaved = -1;
        printf("proc_leave_room fail: room_id invalid\n");
        return;
    } else {
        //leave room
        isLeaved = leaveRoom(leave_room->room_id, cliIdx);
    }
    //set body
    //fail  result:RSP_VALUE_FAIL   admin:NULL string
    if (isLeaved == -1) {
        rspMsg.body.uniRsp.leave_room.result = RSP_VALUE_FAIL;
        rspMsg.body.uniRsp.leave_room.admin[0] = '\0';
    }
    //success and admin not change  result:RSP_VALUE_SUCC   admin:NULL string
    else if(isLeaved == 1) {
        rspMsg.body.uniRsp.leave_room.result = RSP_VALUE_SUCC;
        rspMsg.body.uniRsp.leave_room.admin[0] = '\0';
    }
    //success and admin not change  result:RSP_VALUE_SUCC   admin:new admin's nickname
    else {
        rspMsg.body.uniRsp.leave_room.result = RSP_VALUE_SUCC;
        strcpy(rspMsg.body.uniRsp.leave_room.admin, _client[_gameRoom[leave_room->room_id-1].admin].nickname);
    }
    //set header
    setHeader(&rspMsg.header, MSG_LEAVE_ROOM, sizeof(stLeave_room_rsp));
    //send result
    int sendRet = writeMsg(_client[cliIdx].socket, &rspMsg, 500);
    if(sendRet == -1) printf("rsp_leave_room fail\n");
}

void proc_handover_admin(message *msg, int cliIdx)
{
    //extract body from msg
    stHandover_admin_rpt *handover = &msg->body.uniRpt.handover_admin;

    int roomIdx = handover->room_id-1;
    message castMsg;
    int isHandovered;
    //check valid
    if(IsRoomUsed(handover->room_id) == 0) {
        printf("handover_admin fail: room_id invalid\n");
        return;
    }
    if(cliIdx != _gameRoom[roomIdx].admin) {
        printf("handover_admin fail: msg not from admin\n");
        return;
    }

    //handover admin
    int newAdmin = findUserByName(roomIdx, handover->newAdmin);
    if(newAdmin == -1) {
        isHandovered = -1;
    } else {
        isHandovered = handoverAdmin(handover->room_id, newAdmin);
    }

    //broadcast control message, include messege sender
    if(isHandovered == 1) {
        int broadcastRst = broadcastMsg(&castMsg, roomIdx, -1);
        if(broadcastRst == -1) printf("broadcast handover admin fail.\n");
    }
}

void proc_kick_user(message *msg, int cliIdx)
{
    //extract body from msg
    stKick_user_rpt *kick = &msg->body.uniRpt.kick_user;

    int roomIdx = kick->room_id-1;
    message castMsg;
    int isKicked;
    //check valid
    if(IsRoomUsed(kick->room_id) == 0) {
        printf("kick_user fail: room_id invalid\n");
        return;
    }
    if(cliIdx != _gameRoom[roomIdx].admin) {
        printf("kick_user fail: msg not from admin\n");
        return;
    }

    //kick user
    int kickTarget = findUserByName(roomIdx, kick->kickTarget);
    if(kickTarget == -1) {
        isKicked = -1;
    } else {
        isKicked = handoverAdmin(kick->room_id, kickTarget);
    }

    //broadcast control message, include messege sender
    if(isKicked == 1) {
        int broadcastRst = broadcastMsg(&castMsg, roomIdx, -1);
        if(broadcastRst == -1) printf("broadcast handover admin fail.\n");
    }
}

void proc_room_list(stRoom_list_req *room_list, int cliIdx)
{
    message rspMsg;
    stRoomList retRoomList;
    // set room list
    int isNext = roomList(room_list->page, &retRoomList);
    // set body
    if(isNext == -1) {
        rspMsg.body.uniRsp.room_list.isNext = RSP_QRY_FAIL;
    } else {
        if(isNext == 1) rspMsg.body.uniRsp.room_list.isNext = RSP_QRY_ISNEXT;
        else rspMsg.body.uniRsp.room_list.isNext = RSP_QRY_NONEXT;
        memcpy(&rspMsg.body.uniRsp.room_list, &retRoomList, sizeof(stRoomList));
    }
    // set header
    setHeader(&rspMsg.header, MSG_QRY_ROOM_LIST, sizeof(stRoom_list_rsp));
    // send result 
    int sendRet = writeMsg(_client[cliIdx].socket, &rspMsg, 500);
    if(sendRet == -1) printf("rsp_room_list fail\n");
}

void proc_position(message *msg, int cliIdx)
{
    //extract body from msg
    stPosition_rpt *position = &msg->body.uniRpt.position;
    int roomIdx = position->room_id - 1;
    //check valid
    if(IsRoomUsed(position->room_id) == 0) {
        printf("proc_position fail: room_id invalid\n");
        return;
    }

    //broadcast position message
    int broadcastRst = broadcastMsg(msg, roomIdx, cliIdx);
    if(broadcastRst == -1) printf("broadcast position of user %s fail.\n", _client[cliIdx].nickname);
}

void proc_catch_runner(stCatch_req *catch, int cliIdx)
{
    message rspMsg;
    int roomIdx = catch->room_id - 1;
    //check valid
    if(IsRoomUsed(catch->room_id) == 0) {
        printf("catch fail: room_id invalid\n");
        return;
    }
    int targetIdx = checkNicknameValid(catch->room_id, catch->catchWhom);
    if(targetIdx == -1) {
        printf("catch fail: target nickname invalid\n");
        return;
    }

    //catch runner
    int catchRst = catchRunner(catch->room_id, cliIdx);

    //process catch result
    //catch target invalid
    if(catchRst == -1) {
        printf("catch fail: target nickname invalid\n");
        return;
    }
    //return of catchRunner == 0 (no more survive runner)
    else if(catchRst == 0) {
        /*game end*/
        //set header
        setHeader(&rspMsg.header, MSG_GAME_END, sizeof(stGame_end_rsp));
        //set body
        rspMsg.body.uniRsp.game_end.totalTime = calcTotalTime(roomIdx);
    } 
    //return of catchRunner > 0 (runner survive)
    else {
        //set header
        setHeader(&rspMsg.header, MSG_CATCH, sizeof(stCatch_rsp));
        //set body
        strcpy(rspMsg.body.uniRsp.catch.catchWhom, catch->catchWhom);
        //broadcast catch message
        int broadcastRst = broadcastMsg(&rspMsg, roomIdx, cliIdx);
    }
}

