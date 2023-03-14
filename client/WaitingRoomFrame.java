import javax.swing.*;
import java.awt.*;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;

public class WaitingRoomFrame extends JFrame {
    SocketHandler handler;
    Values values;
    JPanel sidePanel = new JPanel();
    Chat chat;
    CenterPanel centerPanel;
    GameOptions gameOptions;
    WaitingRoom waitingRoom;
    WaitingRoomFrame(SocketHandler handler, Values values) {
        values.readValues.startListener = startListener;
        values.readValues.adminListener = adminListener;
        this.handler = handler;
        this.values = values;
        this.setFocusable(false);

        waitingRoom = new WaitingRoom(startSignal, optionPanelListener, handler, values);

        gameOptions = new GameOptions(values);
        gameOptions.setVisible(false);
        centerPanel = new CenterPanel(focusListener, startSignal, optionPanelListener);


        chat = new Chat(centerPanel);
        chat.setVisible(true);
        sidePanel.add(chat);
        sidePanel.add(gameOptions);

        setTitle("HideNSeek");                                 //제목
        add(waitingRoom, BorderLayout.WEST);
        add(centerPanel, BorderLayout.CENTER);
        add(sidePanel, BorderLayout.EAST);

        setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);        //창 닫으면 종료
        setResizable(false);                                   //창 크기 조절 불가능
        pack();                                                //내용물에 맞게 창 크기 조절
        setVisible(true);                                      //창을 보이게 설정
        setLocationRelativeTo(null);                           //창 위치를 화면 중앙으로
        waitingRoom.grabFocus();
        adminListener.actionPerformed(null);
    }

    ActionListener startSignal = new ActionListener() {
        @Override
        public void actionPerformed(ActionEvent e) {
            values.writeValues.msgType = MsgType.MSG_GAME_START;
            values.writeValues.writeListener.actionPerformed(null);
            new Waiting(handler, values);
            setVisible(false);
        }
    };
    ActionListener optionPanelListener = new ActionListener() {
        @Override
        public void actionPerformed(ActionEvent e) {
            if(chat.isVisible()) {
                chat.setVisible(false);
                gameOptions.setVisible(true);
            } else {
                chat.setVisible(true);
                gameOptions.setVisible(false);
            }
            pack();
        }
    };
    ActionListener focusListener = new ActionListener() {
        @Override
        public void actionPerformed(ActionEvent e) {
            waitingRoom.grabFocus();
            centerPanel.actionPerformed(null);
        }
    };
    ActionListener startListener = new ActionListener() {
        @Override
        public void actionPerformed(ActionEvent e) {
            waitingRoom.timer.stop();
            new InGameFrame(handler, values);
            dispose();
        }
    };

    ActionListener adminListener = new ActionListener() {
        @Override
        public void actionPerformed(ActionEvent e) {
            if(values.staticValues.adminColorIdx != values.staticValues.colorIdx) {
                centerPanel.notAdmin.actionPerformed(null);
                gameOptions.notAdmin.actionPerformed(null);
            } else {
                centerPanel.admin.actionPerformed(null);
                gameOptions.admin.actionPerformed(null);
            }
        }
    };
}
