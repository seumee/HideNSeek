import java.util.concurrent.Executors;

public class HideNSeek {
    public static void main(String[] args) {
        String serverIp = "192.168.254.16";
        int port = 5881;
        Values values = new Values();
        values.writeValues.msgType = MsgType.MSG_HELLO;
        SocketHandler handler = new SocketHandler(serverIp, port, values);
        Executors.newSingleThreadExecutor().execute(handler);
        new Login(handler, values);
    }
}