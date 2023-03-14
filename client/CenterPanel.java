import javax.swing.*;
import javax.swing.plaf.metal.MetalButtonUI;
import java.awt.*;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;

public class CenterPanel extends JPanel implements ActionListener{
    static final int PANEL_WIDTH = 90;   //가로크기
    static final int PANEL_HEIGHT = 480;   //세로크기
    ActionListener focusListener;
    ActionListener startListener;
    ActionListener optionListener;
    ImageIcon normalOption = newIcon("settings_full");
    ImageIcon pressedOption = newIcon("settings_empty");
    ImageIcon normalChat = newIcon("chat_full");
    ImageIcon pressedChat = newIcon("chat_empty");
    ImageIcon normalStart = newIcon("start_full");
    ImageIcon pressedStart = newIcon("start_empty");
    ImageIcon normalFocus = newIcon("left_full");
    ImageIcon pressedFocus = newIcon("left_empty");
    JButton focusBtn = new JButton("", normalFocus);
    JButton optionBtn = new JButton("", normalOption);
    boolean optionToggle = true;
    JButton startBtn = new JButton("", normalStart);
    JButton focusBtnLabel = new JButton("Focus");
    JButton optionBtnLabel = new JButton("(O)");
    JButton startBtnLabel = new JButton("(SpaceBar)");
    private ImageIcon newIcon(String fileName) {
        ImageIcon icon = new ImageIcon(InGame.class.getResource("icons/"+fileName+".png"));
        return getScaledIcon(icon, 50, 50);
    }
    private ImageIcon getScaledIcon(ImageIcon original, int w, int h) {
        Image tmpImg = original.getImage();
        ImageIcon scaledIcon = new ImageIcon(tmpImg.getScaledInstance(w, h, Image.SCALE_SMOOTH));
        return scaledIcon;
    }
    public CenterPanel(ActionListener focusListener, ActionListener startListener, ActionListener optionListener) {
        this.focusListener = focusListener;
        this.startListener = startListener;
        this.optionListener = optionListener;

        setPreferredSize(new Dimension(PANEL_WIDTH, PANEL_HEIGHT));
        setLayout(null);
        setBounds(0,0,PANEL_WIDTH, PANEL_HEIGHT);
        setBackground(Color.WHITE);
        setFocusable(false);

        setButtons();
    }

    private void setButtons() {
        optionBtn.setPressedIcon(pressedOption);
        setBtnDefault(optionBtn, 15, 20, 60, 50);
        optionBtn.addActionListener(optionListener);
        optionBtn.addActionListener(new ActionListener() {
            @Override
            public void actionPerformed(ActionEvent e) {
                if(optionToggle) {
                    optionBtn.setIcon(normalChat);
                    optionBtn.setPressedIcon(pressedChat);
                } else {
                    optionBtn.setIcon(normalOption);
                    optionBtn.setPressedIcon(pressedOption);
                }
                optionToggle = !optionToggle;
            }
        });
        optionBtnLabel.setEnabled(false);
        setBtnDefault(optionBtnLabel, 20, 70, 90, 20);

        focusBtn.setPressedIcon(pressedFocus);
        setBtnDefault(focusBtn, 15, 120, 60, 50);
        focusBtn.addActionListener(focusListener);
        focusBtnLabel.setEnabled(false);
        setBtnDefault(focusBtnLabel, 10, 170, 90, 20);
        focusBtn.setVisible(false);
        focusBtnLabel.setVisible(false);

        startBtn.setPressedIcon(getScaledIcon(pressedStart, 50, 50));
        setBtnDefault(startBtn, 15, PANEL_HEIGHT -90, 60, 50);
        startBtn.addActionListener(startListener);
        startBtnLabel.setEnabled(false);
        setBtnDefault(startBtnLabel, 0, PANEL_HEIGHT -40, 100, 40);
    }
    private void setBtnDefault(JButton btn, int x, int y, int w, int h) {
        btn.setUI(new MetalButtonUI() {
            protected Color getDisabledTextColor() {
                return Color.BLACK;
            }
        });
        btn.setFocusable(false);
        btn.setBorderPainted(false);
        btn.setContentAreaFilled(false);
        btn.setBounds(x, y, w, h);
        add(btn);
    }
    @Override
    public void actionPerformed(ActionEvent event) {
        boolean toVisible = !focusBtn.isVisible();
        focusBtn.setVisible(toVisible);
        focusBtnLabel.setVisible(toVisible);
    }
    ActionListener notAdmin = new ActionListener() {
        @Override
        public void actionPerformed(ActionEvent e) {
            startBtn.setVisible(false);
            startBtnLabel.setVisible(false);
        }
    };
    ActionListener admin = new ActionListener() {
        @Override
        public void actionPerformed(ActionEvent e) {
            startBtn.setVisible(true);
            startBtnLabel.setVisible(true);
        }
    };
}
