import javax.swing.*;
import java.awt.*;

public class Waiting extends JFrame {
    private JPanel waitingPanel = new JPanel(new GridLayout(2, 1));

    private JLabel label = new JLabel("서버 응답을 기다리는 중입니다...");
    SocketHandler handler;
    Values values;
    MsgType msg = MsgType.MSG_HELLO;

    public Waiting(SocketHandler handler, Values values) {
        super("HideNSeek");
        this.handler = handler;
        this.values = values;

        label.setHorizontalAlignment(NORMAL);
        waitingPanel.add(label);
        setContentPane(waitingPanel);

        setSize(350, 150);
        setLocationRelativeTo(null);
        setVisible(true);
        setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);

        while(msg.MsgType(values.readValues.msgType) == MsgType.MIN_MSG_VALUE) {}
        switch (msg.MsgType(values.readValues.msgType)) {
            case MSG_LOGIN -> {
                requestRoomList(1);
            }
            case GET_PAGE -> {
                requestRoomList(values.writeValues.page);
            }
            case MSG_JOIN_ROOM -> {
                System.out.println(values.readValues.result);
                if (values.readValues.result == Values.RSP_VALUE_FULL_ROOM) {
                    JOptionPane.showMessageDialog(null, "Full room");
                    requestRoomList(1);
                } else if (values.readValues.result == Values.RSP_VALUE_RUNNING_ROOM) {
                    JOptionPane.showMessageDialog(null, "This room is already running.");
                    requestRoomList(1);
                }else new WaitingRoomFrame(handler, values);
            }
            case MSG_GAME_START -> {
                if(values.readValues.result == Values.RSP_VALUE_FAIL) {
                    JOptionPane.showMessageDialog(null, "Game Start Failed.");
                    new WaitingRoomFrame(handler, values);
                }
            }
            default -> {
            }
        }
        setVisible(false);
    }

    private void requestRoomList(int page) {
        values.writeValues.page = page;
        values.writeValues.msgType = MsgType.MSG_QRY_ROOM_LIST;
        values.writeValues.writeListener.actionPerformed(null);
        while(this.msg.MsgType(this.values.readValues.msgType) != this.msg.MSG_QRY_ROOM_LIST) {}
        new LobbyFrame(this.handler, values);
        setVisible(false);
    }
}
