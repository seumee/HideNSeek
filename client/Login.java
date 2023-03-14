import javax.swing.*;
import java.awt.*;
import java.awt.event.ActionEvent;
import java.awt.event.KeyEvent;

public class Login extends JFrame {
    private JPanel loginPanel = new JPanel(new GridLayout(3, 1));

    private JLabel label = new JLabel("게임에서 사용할 닉네임을 입력하세요.");

    private JTextField nickname = new JTextField();
    private JButton loginBtn = new JButton("Login");
    public Login(SocketHandler handler, Values values) {
        super("HideNSeek");
        this.setContentPane(loginPanel);
        loginPanel.add(label);
        loginPanel.add(nickname);
        loginPanel.add(loginBtn);

        label.setHorizontalAlignment(NORMAL);
        setVisible(false);

        setVisible(true);
        setSize(350, 150);
        setLocationRelativeTo(null);
        setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);

        Action login = new AbstractAction() {
            @Override
            public void actionPerformed(ActionEvent e) {
                values.staticValues.nickname = nickname.getText().trim();
                values.writeValues.msgType = MsgType.MSG_LOGIN;
                values.writeValues.writeListener.actionPerformed(null);

                new Waiting(handler, values);
                //setVisible(false);
                dispose();
            }
        };

        KeyStroke enter = KeyStroke.getKeyStroke(KeyEvent.VK_ENTER, 0, false);
        nickname.getInputMap(JTable.WHEN_ANCESTOR_OF_FOCUSED_COMPONENT).put(enter, "ENTER");
        nickname.getActionMap().put("ENTER", login);
        loginBtn.addActionListener(login);
    }
}
