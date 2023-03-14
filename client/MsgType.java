public enum MsgType {
    MIN_MSG_VALUE,
    MSG_LOGIN,
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
    MSG_GAME_END,
    MSG_BLOCK,
    MSG_CHAT,
    MSG_HELLO,
    MSG_BYE,
    MAX_MSG_VALUE,
    ///////////////////////below messages are for client use.
    GET_PAGE;

    MsgType MsgType(int value) {
        return MsgType.values()[value];
    }
}

