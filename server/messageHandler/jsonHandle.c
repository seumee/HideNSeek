#include "networkFunc/myNP.h"

// success: 1,   fail: 0
int json_decode_login_req(json_object* msgObj, stLogin_req* stReq) {
    json_object *nickname;

    nickname = json_object_object_get(msgObj, "nickname");
    if(nickname != NULL) {
        strcpy(stReq->nickname, json_object_get_string(nickname));
    } else return 0;
    return 1;
}

int json_encode_login_rsp(stLogin_rsp* stRsp, int cliIdx) {
    json_object *msgObj;
    char jsonStr[MAX_BUFFER_SIZE] = {0,};

    msgObj = json_object_new_object();

    //set message content
    json_object_object_add(msgObj, "msgType", json_object_new_int(MSG_LOGIN));
    json_object_object_add(msgObj, "result", json_object_new_int(stRsp->result));
    strcpy(jsonStr, json_object_to_json_string(msgObj));
    jsonStr[strlen(jsonStr)] = '\n';
    //send message
    int sendRet = writeMsg(_client[cliIdx].socket, jsonStr, 500);
    if(sendRet == -1) printf("rsp_login fail\n");
    return 1;
}

// success: 1,   fail: 0
int json_decode_crt_room_req(json_object* msgObj, stCrt_room_req* stReq) {
    json_object *roomname;

    roomname = json_object_object_get(msgObj, "roomname");
    if(roomname != NULL) {
        strcpy(stReq->roomname, json_object_get_string(roomname));
    } else return 0;
    return 1;
}

int json_encode_crt_room_rsp(stCrt_room_rsp* stRsp, int cliIdx) {
    json_object *msgObj;
    char jsonStr[MAX_BUFFER_SIZE] = {0,};

    msgObj = json_object_new_object();

    //set message content
    json_object_object_add(msgObj, "msgType", json_object_new_int(MSG_CRT_ROOM));
    json_object_object_add(msgObj, "result", json_object_new_int(stRsp->result));
    json_object_object_add(msgObj, "room_id", json_object_new_int(stRsp->room_id));
    strcpy(jsonStr, json_object_to_json_string(msgObj));
    jsonStr[strlen(jsonStr)] = '\n';
    //send message
    int sendRet = writeMsg(_client[cliIdx].socket, jsonStr, 500);
    if(sendRet == -1) printf("rsp_crt_room fail\n");
    return 1;
}

// success: 1,   fail: 0
int json_decode_join_room_req(json_object* msgObj, stJoin_room_req* stReq) {
    json_object *roomId;

    roomId = json_object_object_get(msgObj, "room_id");
    if(roomId != NULL) {
        stReq->room_id = json_object_get_int(roomId);
    } else return 0;
    return 1;
}

int json_encode_join_room_rsp(stJoin_room_rsp* stRsp, int cliIdx) {
    json_object *msgObj, *options;
    char jsonStr[MAX_BUFFER_SIZE] = {0,};
    int ret;

    msgObj = json_object_new_object();

    //set message content
    json_object_object_add(msgObj, "msgType", json_object_new_int(MSG_JOIN_ROOM));
    json_object_object_add(msgObj, "result", json_object_new_int(stRsp->result));
    
    if(stRsp->result != RSP_VALUE_FAIL) {
        options = json_object_new_object();

        json_object_object_add(msgObj, "colorIdx", json_object_new_int(stRsp->colorIdx));
        json_object_object_add(msgObj, "adminColorIdx", json_object_new_int(stRsp->adminColorIdx));

        json_object_object_add(options, "numberOfTagger", json_object_new_int(stRsp->options.numberOfTagger));
        json_object_object_add(options, "gameSpeed", json_object_new_int(stRsp->options.gameSpeed));
        json_object_object_add(options, "sight_disadvantage", json_object_new_int(stRsp->options.sight_disadvantage));
        json_object_object_add(options, "taggerSpeed", json_object_new_int(stRsp->options.taggerSpeed));
        json_object_object_add(options, "catch_cooldown", json_object_new_int(stRsp->options.catch_cooldown));
        json_object_object_add(options, "block_cooldown", json_object_new_int(stRsp->options.block_cooldown));
        json_object_object_add(msgObj, "game_option", options);
    } 

    strcpy(jsonStr, json_object_to_json_string(msgObj));
    jsonStr[strlen(jsonStr)] = '\n';
    //send message
    if(stRsp->result != RSP_VALUE_SUCC) {  //if join_room failed
        ret = writeMsg(_client[cliIdx].socket, jsonStr, 500);
    } else {    //broadcast message
        ret = broadcastMsg(jsonStr, _client[cliIdx].room_id-1, -1);
    }
    if(ret == -1) printf("rsp_join_room fail\n");
    return 1;
}

// success: 1,   fail: 0
int json_decode_config_room_rpt(json_object* msgObj, stConfig_room_rpt* stRpt) {
    json_object *roomId, *gameOptions;

    roomId = json_object_object_get(msgObj, "room_id");
    gameOptions = json_object_object_get(msgObj, "game_option");
    if(roomId != NULL) {
        stRpt->room_id = json_object_get_int(roomId);
        printf("room_id: %d\n", stRpt->room_id);
    } else return 0;
    if(gameOptions != NULL) {
        stRpt->options.numberOfTagger = json_object_get_int(json_object_object_get(gameOptions, "numberOfTagger"));
        stRpt->options.gameSpeed = json_object_get_int(json_object_object_get(gameOptions, "gameSpeed"));
        stRpt->options.sight_disadvantage = json_object_get_int(json_object_object_get(gameOptions, "sight_disadvantage"));
        stRpt->options.taggerSpeed = json_object_get_int(json_object_object_get(gameOptions, "taggerSpeed"));
        stRpt->options.catch_cooldown = json_object_get_int(json_object_object_get(gameOptions, "catch_cooldown"));
        stRpt->options.block_cooldown = json_object_get_int(json_object_object_get(gameOptions, "block_cooldown"));
        stRpt->room_id = json_object_get_int(roomId);
    } else return 0;
    return 1;
}

// success: 1,   fail: 0
int json_decode_game_start_req(json_object* msgObj, stGame_start_req* stReq) {
    json_object *roomId;

    roomId = json_object_object_get(msgObj, "room_id");
    if(roomId != NULL) {
        stReq->room_id = json_object_get_int(roomId);
    } else return 0;
    return 1;
}

int json_encode_game_start_rsp(stGame_start_rsp* stRsp, int cliIdx) {
    json_object *msgObj, *role;
    char jsonStr[MAX_BUFFER_SIZE] = {0,};
    int ret;

    msgObj = json_object_new_object();
    role = json_object_new_array();

    //set message content
    json_object_object_add(msgObj, "msgType", json_object_new_int(MSG_GAME_START));
    json_object_object_add(msgObj, "result", json_object_new_int(stRsp->result));
    for(int i = 0; i < MAX_ROOM_SIZE; i++) {
        json_object_array_add(role, json_object_new_int(stRsp->role[i]));
    }
    json_object_object_add(msgObj, "role", role);
    strcpy(jsonStr, json_object_to_json_string(msgObj));
    jsonStr[strlen(jsonStr)] = '\n';
    if(stRsp->result == RSP_VALUE_FAIL) {  //if game_start failed
        ret = writeMsg(_client[cliIdx].socket, jsonStr, 500);
    } else {    //broadcast message
        ret = broadcastMsg(jsonStr, _client[cliIdx].room_id-1, -1);
    }
    if(ret == -1) printf("rsp_game_start fail\n");
    return 1;
}

// success: 1,   fail: 0
int json_decode_game_ctrl_rpt(json_object* msgObj, stGame_ctrl_rpt* stRpt) {
    json_object *roomId, *status;

    roomId = json_object_object_get(msgObj, "room_id");
    status = json_object_object_get(msgObj, "status");
    if(roomId != NULL) {
        stRpt->room_id = json_object_get_int(roomId);
    } else return 0;
    if(status != NULL) {
        stRpt->status = json_object_get_int(status);
    } else return 0;
    return 1;
}

// success: 1,   fail: 0
int json_decode_del_room_req(json_object* msgObj, stDel_room_req* stReq) {
    json_object *roomId;

    roomId = json_object_object_get(msgObj, "room_id");
    if(roomId != NULL) {
        stReq->room_id = json_object_get_int(roomId);
    } else return 0;
    return 1;
}

int json_encode_del_room_rsp(stDel_room_rsp* stRsp, int cliIdx) {
    json_object *msgObj;
    char jsonStr[MAX_BUFFER_SIZE] = {0,};
    int ret;

    msgObj = json_object_new_object();

    //set message content
    json_object_object_add(msgObj, "msgType", json_object_new_int(MSG_DEL_ROOM));
    json_object_object_add(msgObj, "result", json_object_new_int(stRsp->result));
    strcpy(jsonStr, json_object_to_json_string(msgObj));
    jsonStr[strlen(jsonStr)] = '\n';
    if(stRsp->result == RSP_VALUE_FAIL) {  //if game_start failed
        ret = writeMsg(_client[cliIdx].socket, jsonStr, 500);
    } else {    //broadcast message
        ret = broadcastMsg(jsonStr, _client[cliIdx].room_id-1, -1);
    }
    if(ret == -1) printf("rsp_del_room fail\n");
    return 1;
}

// success: 1,   fail: 0
int json_decode_leave_room_req(json_object* msgObj, stLeave_room_req* stReq) {
    json_object *roomId;

    roomId = json_object_object_get(msgObj, "room_id");
    if(roomId != NULL) {
        stReq->room_id = json_object_get_int(roomId);
    } else return 0;
    return 1;
}

int json_encode_leave_room_rsp(stLeave_room_rsp* stRsp, int cliIdx, int roomId) {
    printf("json_encode_leave_room proc.\n");
    json_object *msgObj;
    char jsonStr[MAX_BUFFER_SIZE] = {0,};
    int ret;

    msgObj = json_object_new_object();

    //set message content
    json_object_object_add(msgObj, "msgType", json_object_new_int(MSG_LEAVE_ROOM));
    json_object_object_add(msgObj, "whoLeave", json_object_new_int(stRsp->whoLeave));
    if(stRsp->whoLeave != RSP_VALUE_FAIL) {
        json_object_object_add(msgObj, "adminColorIdx", json_object_new_int(stRsp->adminColorIdx));
    }

    strcpy(jsonStr, json_object_to_json_string(msgObj));
    jsonStr[strlen(jsonStr)] = '\n';
    if(stRsp->whoLeave == RSP_VALUE_FAIL) {  //if leave_room failed
        printf("leave_room_write.\n");
        ret = writeMsg(_client[cliIdx].socket, jsonStr, 500);
    } else {    //broadcast message
        printf("leave_room_broadcast.\n");
        ret = broadcastMsg(jsonStr, roomId-1, -1);
    }
    if(ret == -1) printf("rsp_leave_room fail\n");
    return 1;

}

// success: 1,   fail: 0
int json_decode_handover_admin_rpt(json_object* msgObj, stHandover_admin_rpt* stRpt) {
    json_object *roomId, *newadmin;

    roomId = json_object_object_get(msgObj, "room_id");
    newadmin = json_object_object_get(msgObj, "newadmin");
    if(roomId != NULL) {
        stRpt->room_id = json_object_get_int(roomId);
    } else return 0;
    if(newadmin != NULL) {
        strcpy(stRpt->newAdmin, json_object_get_string(newadmin));
    } else return 0;
    return 1;
}

// success: 1,   fail: 0
int json_decode_kick_user_rpt(json_object* msgObj, stKick_user_rpt* stRpt) {
    json_object *roomId, *kicktarget;

    roomId = json_object_object_get(msgObj, "room_id");
    kicktarget = json_object_object_get(msgObj, "kicktarget");
    if(roomId != NULL) {
        stRpt->room_id = json_object_get_int(roomId);
    } else return 0;
    if(kicktarget != NULL) {
        strcpy(stRpt->kickTarget, json_object_get_string(kicktarget));
    } else return 0;
    return 1;
}

// success: 1,   fail: 0
int json_decode_room_list_req(json_object* msgObj, stRoom_list_req* stReq) {
    json_object *page;

    page = json_object_object_get(msgObj, "page");
    if(page != NULL) {
        stReq->page = json_object_get_int(page);
    } else return 0;
    return 1;
}

int json_encode_room_list_rsp(stRoom_list_rsp* stRsp, int cliIdx) {
    json_object *msgObj, *roomListObj, *roomid_list, *roomname_list, *usercount_list;
    char jsonStr[MAX_BUFFER_SIZE] = {0,};
    int ret;

    msgObj = json_object_new_object();
    roomListObj = json_object_new_object();
    roomid_list = json_object_new_array();
    roomname_list = json_object_new_array();
    usercount_list = json_object_new_array();

    //set message content
    json_object_object_add(msgObj, "msgType", json_object_new_int(MSG_QRY_ROOM_LIST));
    json_object_object_add(msgObj, "isnext", json_object_new_int(stRsp->isNext));
    json_object_object_add(msgObj, "page", json_object_new_int(stRsp->page));

    for(int i = 0; stRsp->room_list.roomName_list[i][0] != '\0'; i++) {        
        json_object_array_add(roomid_list, json_object_new_int(stRsp->room_list.roomId_list[i]));
        json_object_array_add(roomname_list, json_object_new_string(stRsp->room_list.roomName_list[i]));
        json_object_array_add(usercount_list, json_object_new_int(stRsp->room_list.userCount_list[i]));
    }
    json_object_object_add(roomListObj, "roomid_list", roomid_list);
    json_object_object_add(roomListObj, "roomname_list", roomname_list);
    json_object_object_add(roomListObj, "usercount_list", usercount_list);
    json_object_object_add(msgObj, "roomlist", roomListObj);
    strcpy(jsonStr, json_object_to_json_string(msgObj));
    jsonStr[strlen(jsonStr)] = '\n';

    //send message
    int sendRet = writeMsg(_client[cliIdx].socket, jsonStr, 500);
    if(sendRet == -1) printf("rsp_room_list fail\n");
    return 1;
}

//need only roomId, nickname
//isAlive가 필요한가,,,?
int json_decode_position_rpt(json_object* msgObj, stPosition_rpt* stRpt) {
    json_object *roomId, *nickname;

    roomId = json_object_object_get(msgObj, "room_id");
    nickname = json_object_object_get(msgObj, "nickname");
    if(roomId != NULL) {
        stRpt->room_id = json_object_get_int(roomId);
    } else return 0;
    if(nickname != NULL) {
        strcpy(stRpt->nickname, json_object_get_string(nickname));
    } else return 0;
    return 1;
}

int json_decode_catch_rpt(json_object* msgObj, stCatch_rpt* stRpt) {
    json_object *roomId, *catchwhom, *tagger;

    roomId = json_object_object_get(msgObj, "room_id");
    catchwhom = json_object_object_get(msgObj, "catchwhom");
    tagger = json_object_object_get(msgObj, "tagger");
    if(roomId != NULL) {
        stRpt->room_id = json_object_get_int(roomId);
    } else return 0;
    if(catchwhom != NULL) {
        stRpt->catchWhom = json_object_get_int(catchwhom);
    } else return 0;
    if(tagger != NULL) {
        stRpt->tagger = json_object_get_int(tagger);
    } else return 0;
    return 1;
}

int json_decode_block_rpt(json_object* msgObj, stBlock_rpt* stRpt) {
    json_object *roomId, *tagger;

    roomId = json_object_object_get(msgObj, "room_id");
    tagger = json_object_object_get(msgObj, "tagger");
    if(roomId != NULL) {
        stRpt->room_id = json_object_get_int(roomId);
    } else return 0;
    if(tagger != NULL) {
        stRpt->tagger = json_object_get_int(tagger);
    } else return 0;
    return 1;
}

// success: 1,   fail: 0
int json_decode_game_end_req(json_object* msgObj, stGame_end_req* stReq) {
    json_object *roomId;

    roomId = json_object_object_get(msgObj, "room_id");
    if(roomId != NULL) {
        stReq->room_id = json_object_get_int(roomId);
    } else return 0;
    return 1;
}

int json_encode_game_end_rsp(stGame_end_rsp* stRsp, int cliIdx) {
    json_object *msgObj;
    char jsonStr[MAX_BUFFER_SIZE] = {0,};
    int ret;

    msgObj = json_object_new_object();

    //set message content
    json_object_object_add(msgObj, "msgType", json_object_new_int(MSG_GAME_END));
    json_object_object_add(msgObj, "totaltime", json_object_new_int(stRsp->totalTime));
    strcpy(jsonStr, json_object_to_json_string(msgObj));
    jsonStr[strlen(jsonStr)] = '\n';
    ret = broadcastMsg(jsonStr, _client[cliIdx].room_id-1, -1);

    if(ret == -1) printf("broadcast game_end fail\n");
    return 1;
}