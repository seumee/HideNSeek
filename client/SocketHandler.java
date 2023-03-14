import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.io.IOException;
import java.net.InetSocketAddress;
import java.net.Socket;
import java.net.SocketAddress;
import java.nio.ByteBuffer;
import java.nio.channels.*;
import org.json.simple.*;
import org.json.simple.parser.JSONParser;
import org.json.simple.parser.ParseException;

public class SocketHandler implements Runnable{
    private InetSocketAddress connectAddress;
    private Values values;

    // ip와 port 설정
    public SocketHandler(String address, int port, Values values) {
        this.values = values;
        connectAddress = new InetSocketAddress(address, port);
        System.out.println("run success");
    }

    // Thread 실행.
    public void run() {
        Thread writer;
        try (SocketChannel channel = SocketChannel.open(connectAddress)) {
            channel.configureBlocking(false);
            ByteBuffer buffer = ByteBuffer.allocate(8192*2);
            Socket socket = channel.socket();
            StringBuffer sb = new StringBuffer();

            writer = new Thread(new Writer(channel));
            writer.start();

            while (true) {
                int size = channel.read(buffer);

                if (size == -1) {
                    SocketAddress remoteAddr = socket.getRemoteSocketAddress();
                    System.out.println("Connection closed by client: " + remoteAddr);
                    // 소켓 채널 닫기
                    channel.close();
                    // 소켓 닫기
                    socket.close();
                    return;
                } else if(size > 0) {
                    // ByteBuffer -> byte[]
                    byte[] data = new byte[size];
                    System.arraycopy(buffer.array(), 0, data, 0, size);
                    buffer.clear();

                    // 버퍼에 수신된 데이터 추가
                    sb.append(new String(data));
                    for(int i = 0; i < sb.length(); i++) {
                        if(sb.charAt(i) == '\n') {
                            String msg = sb.substring(0, i);
                            sb.delete(0, i + 1);
                            messageHandle(msg);
                        }
                    }

                }
            }

        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    class Writer implements Runnable {
        SocketChannel channel;
        Writer(SocketChannel channel) {
            System.out.println("Writer created");
            this.channel = channel;
        }

        @Override
        public void run() {
            JSONObject obj = new JSONObject();
            class setContentListener implements ActionListener {
                @Override
                public void actionPerformed(ActionEvent e) {
                    obj.clear();
                    obj.put("msgType", values.writeValues.msgType.ordinal());
                    switch (values.writeValues.msgType) {
                        case MSG_LOGIN -> {
                            System.out.println("MSG_LOGIN Write");
                            obj.put("nickname", values.staticValues.nickname);
                        }
                        case MSG_QRY_ROOM_LIST -> {
                            System.out.println("MSG_QRY_ROOM_LIST Write");
                            obj.put("page", values.writeValues.page);
                        }
                        case MSG_JOIN_ROOM -> {
                            System.out.println("MSG_JOIN_ROOM Write");
                            obj.put("room_id", values.staticValues.roomId);
                        }
                        case MSG_POSITION -> {
                            obj.put("room_id", values.staticValues.roomId);
                            obj.put("nickname", values.staticValues.nickname);
                            obj.put("colorIdx", values.staticValues.colorIdx);
                            if(values.staticValues.isalive) obj.put("isalive", 1);
                            else obj.put("isalive", 0);
                            obj.put("x", values.writeValues.x);
                            obj.put("y", values.writeValues.y);
                        }
                        case MSG_CRT_ROOM -> {
                            System.out.println("MSG_CRT_ROOM Write");
                            obj.put("roomname", values.writeValues.roomname);
                        }
                        case MSG_ROOM_CFG -> {
                            System.out.println("MSG_ROOM_CFG Write");
                            JSONObject options = new JSONObject();
                            obj.put("room_id", values.staticValues.roomId);
                            options.put("numberOfTagger", values.staticValues.options.numberOfTagger);
                            options.put("gameSpeed", values.staticValues.options.gameSpeed);
                            options.put("sight_disadvantage", values.staticValues.options.sight_disadvantage);
                            options.put("taggerSpeed", values.staticValues.options.taggerSpeed);
                            options.put("catch_cooldown", values.staticValues.options.catch_cooldown);
                            options.put("block_cooldown", values.staticValues.options.block_cooldown);
                            obj.put("game_option", options);
                        }
                        case MSG_GAME_START -> {
                            System.out.println("MSG_GAME_START Write");
                            obj.put("room_id", values.staticValues.roomId);
                        }
                        case MSG_BLOCK -> {
                            System.out.println("MSG_BLOCK write");
                            obj.put("room_id", values.staticValues.roomId);
                            obj.put("tagger", values.staticValues.colorIdx);
                            obj.put("x", values.writeValues.blockX);
                            obj.put("y", values.writeValues.blockY);
                        }
                        case MSG_CATCH -> {
                            System.out.println("MSG_CATCH Write");
                            obj.put("room_id", values.staticValues.roomId);
                            obj.put("tagger", values.staticValues.colorIdx);
                            obj.put("catchwhom", values.writeValues.target);
                        }
                        case MSG_GAME_END -> {
                            System.out.println("MSG_GAME_END Write");
                            obj.put("room_id", values.staticValues.roomId);
                        }
                        default -> {}
                    }
                }
            }
            setContentListener setContentListener = new setContentListener();

            class writeListener implements ActionListener {
                @Override
                public void actionPerformed(ActionEvent e) {
                    try {
                        setContentListener.actionPerformed(null);
                        ByteBuffer bb = ByteBuffer.wrap((obj.toJSONString() + '\0').getBytes());
                        channel.write(bb);
                        values.writeValues.msgType = MsgType.MIN_MSG_VALUE;
                    } catch (IOException ie) {
                        ie.printStackTrace();
                    }
                }
            }
            values.writeValues.writeListener = new writeListener();
        }
    }

    private void messageHandle(String jsonMsg) {
        JSONParser parser = new JSONParser();
        if(!jsonMsg.isEmpty()) {
            System.out.println("message: server->client >> "+jsonMsg);
            try {
                JSONObject obj = (JSONObject) parser.parse(jsonMsg);
                values.readValues.msgType = getInt(obj, "msgType");
                MsgType msg = MsgType.MSG_HELLO;

                switch (msg.MsgType(values.readValues.msgType)) {
                    case MSG_LOGIN -> values.readValues.result = getInt(obj, "result");
                    case MSG_QRY_ROOM_LIST -> {
                        for (int i = 0; i < 10; i++) {
                            values.readValues.roomlist.roomId[i] = 0;
                            values.readValues.roomlist.roomName[i] = null;
                        }
                        values.readValues.isnext = getInt(obj, "isnext");
                        values.readValues.page = getInt(obj, "page");
                        JSONObject roomlist = (JSONObject) obj.get("roomlist");
                        JSONArray roomId = (JSONArray) roomlist.get("roomid_list");
                        JSONArray roomName = (JSONArray) roomlist.get("roomname_list");
                        JSONArray userCount = (JSONArray) roomlist.get("usercount_list");

                        for (int i = 0; i < roomId.size(); i++) {
                            values.readValues.roomlist.roomId[i] = (int) ((long) roomId.get(i));
                            values.readValues.roomlist.roomName[i] = (String) roomName.get(i);
                            values.readValues.roomlist.userCount[i] = (int) ((long) userCount.get(i));
                        }
                    }
                    case MSG_JOIN_ROOM -> {
                        int result = getInt(obj, "result");
                        if(result != Values.RSP_VALUE_FAIL) {
                            values.staticValues.userCount = result;
                            int tmpColorIdx = getInt(obj, "colorIdx");
                            if (values.staticValues.colorIdx == -1) {
                                values.staticValues.colorIdx = tmpColorIdx;
                            }
                            JSONObject options = (JSONObject) obj.get("game_option");
                            values.staticValues.options.numberOfTagger = getInt(options, "numberOfTagger");
                            values.staticValues.options.gameSpeed = getInt(options, "gameSpeed");
                            values.staticValues.options.sight_disadvantage = getInt(options, "sight_disadvantage");
                            values.staticValues.options.taggerSpeed = getInt(options, "taggerSpeed");
                            values.staticValues.options.catch_cooldown = getInt(options, "catch_cooldown");
                            values.staticValues.options.block_cooldown = getInt(options, "block_cooldown");

                            values.staticValues.adminColorIdx = getInt(obj, "adminColorIdx");
                            values.staticValues.playerCor[tmpColorIdx][0] = 7;
                            values.staticValues.playerCor[tmpColorIdx][1] = 8;
                        }
                    }
                    case MSG_POSITION -> {
                        int colorIdx = getInt(obj, "colorIdx");
                        if(values.staticValues.roomId == getInt(obj, "room_id")) {
                            values.readValues.nickname = (String) obj.get("nickname");
                            values.readValues.isalive = getInt(obj, "isalive");
                            values.staticValues.playerCor[colorIdx][0] = getInt(obj, "x");
                            values.staticValues.playerCor[colorIdx][1] = getInt(obj, "y");
                        }
                    }
                    case MSG_CRT_ROOM -> {
                        values.staticValues.roomId = getInt(obj, "room_id");
                        values.readValues.result = getInt(obj, "result");
                        if(values.readValues.result == Values.RSP_VALUE_SUCC) {
                            values.staticValues.playerCor[0][0] = 7;
                            values.staticValues.playerCor[0][1] = 8;
                        }
                    }
                    case MSG_ROOM_CFG -> {
                        if(getInt(obj, "room_id") == values.staticValues.roomId) {
                            JSONObject options = (JSONObject) obj.get("game_option");
                            values.staticValues.options.numberOfTagger = getInt(options, "numberOfTagger");
                            values.staticValues.options.gameSpeed = getInt(options, "gameSpeed");
                            values.staticValues.options.sight_disadvantage = getInt(options, "sight_disadvantage");
                            values.staticValues.options.taggerSpeed = getInt(options, "taggerSpeed");
                            values.staticValues.options.catch_cooldown = getInt(options, "catch_cooldown");
                            values.staticValues.options.block_cooldown = getInt(options, "block_cooldown");
                            values.readValues.readListener.actionPerformed(null);
                        }
                        JSONObject options = new JSONObject();
                        obj.put("room_id", values.staticValues.roomId);
                        obj.put("game_option", options);
                    }
                    case MSG_GAME_START -> {
                        values.readValues.result = getInt(obj, "result");
                        if(values.readValues.result == Values.RSP_VALUE_SUCC) {
                            JSONArray role = (JSONArray) obj.get("role");
                            for (int i = 0; i < role.size(); i++) {
                                values.staticValues.roles[i] = (int) ((long) role.get(i));
                            }
                            values.staticValues.amITagger = (values.staticValues.roles[values.staticValues.colorIdx] == Values.ROLE_TAGGER);
                            values.readValues.startListener.actionPerformed(null);
                        }
                    }
                    case MSG_BLOCK -> {
                        if(getInt(obj, "room_id") == values.staticValues.roomId) {
                            values.readValues.blockX = getInt(obj, "x");
                            values.readValues.blockY = getInt(obj, "y");
                            values.readValues.listeners[0].actionPerformed(null);
                        }
                    }
                    case MSG_LEAVE_ROOM -> {
                        int whoLeave = getInt(obj, "whoLeave");
                        if(whoLeave == Values.RSP_VALUE_FAIL) {
                            System.out.println("leave fail.");
                        } else {
                            values.staticValues.userCount--;
                            int newAdmin = getInt(obj, "adminColorIdx");
                            if(newAdmin != -1) values.staticValues.adminColorIdx = newAdmin;
                            values.staticValues.playerCor[whoLeave][0] = 0;
                            values.staticValues.playerCor[whoLeave][1] = 0;
                            values.readValues.adminListener.actionPerformed(null);
                        }
                    }
                    case MSG_CATCH -> {
                        if(getInt(obj, "room_id") == values.staticValues.roomId) {
                            values.readValues.target = getInt(obj, "catchwhom");
                            if(values.readValues.target == values.staticValues.colorIdx) values.staticValues.isalive = false;
                            values.readValues.tagger = getInt(obj, "tagger");
                            values.readValues.listeners[1].actionPerformed(null);
                        }
                    }
                    case MSG_GAME_END -> {
                        values.readValues.totalTime = getInt(obj, "totaltime");
                        values.readValues.gameEndListener.actionPerformed(null);
                    }
                    default -> {}
                }
            } catch (ParseException e) {
            }
        }
    }

    private int getInt(JSONObject jsonObject, String Key) {
        return (int)((long)jsonObject.get(Key));
    }
}
