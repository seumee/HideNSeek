import javax.swing.*;
import javax.swing.plaf.metal.MetalButtonUI;
import java.awt.*;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;

public class StatePanel extends JPanel {
    static final int PANEL_WIDTH = 180;
    static final int PANEL_HEIGHT = 960;   //세로크기

    ImageIcon enabledBlock = newIcon("block_full", 70);
    ImageIcon disabledBlock = newIcon("block_empty", 70);
    ImageIcon enabledCatch = newIcon("catch_full", 70);
    ImageIcon disabledCatch = newIcon("catch_empty", 70);
    ImageIcon statePause = newIcon("pause", 150);
    ImageIcon stateResume = newIcon("resume", 150);

    JButton stateBtn = new JButton("", statePause);
    JButton blockBtn = new JButton("", enabledBlock);
    JButton catchBtn = new JButton("", enabledCatch);

    ActionListener catchSignal;
    private ImageIcon newIcon(String fileName, int size) {
        ImageIcon icon = new ImageIcon(InGame.class.getResource("icons/"+fileName+".png"));
        return getScaledIcon(icon, size, size);
    }
    private ImageIcon getScaledIcon(ImageIcon original, int w, int h) {
        Image tmpImg = original.getImage();
        ImageIcon scaledIcon = new ImageIcon(tmpImg.getScaledInstance(w, h, Image.SCALE_SMOOTH));
        return scaledIcon;
    }

    public StatePanel(Values values, ActionListener catchSignal) {
        ////게임 시간, Tagger&Runner 현황, 일시정지&재개 버튼
        values.staticValues.blockTimerListener = blockListener;
        this.catchSignal = catchSignal;
        setPreferredSize(new Dimension(PANEL_WIDTH, PANEL_HEIGHT));
        setLayout(null);
        setBounds(0,0,PANEL_WIDTH, PANEL_HEIGHT);
        setBackground(Color.WHITE);
        setFocusable(false);
        setVisible(true);

        blockBtn.setFocusable(false);
        blockBtn.setEnabled(false);
        catchBtn.setFocusable(false);
        catchBtn.setEnabled(false);
        stateBtn.setFocusable(false);
        stateBtn.setEnabled(false);

        setBtnDefault(stateBtn, 15, 15, 150, 150);
        setBtnDefault(blockBtn, 15, 200, 70, 70);
        setBtnDefault(catchBtn, 95, 200, 70, 70);

        ///////////catchBtn -> catchSignal actionPerform

        if(values.staticValues.amITagger) {
            blockBtn.setVisible(true);
            catchBtn.setVisible(true);
        } else {
            blockBtn.setVisible(false);
            catchBtn.setVisible(false);
        }
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

    public void setCatchIcon(boolean canCatch) {
        if(canCatch) catchBtn.setIcon(enabledCatch);
        else catchBtn.setIcon(disabledCatch);
    }

    ActionListener blockListener = new ActionListener() {
        @Override
        public void actionPerformed(ActionEvent e) {
            Icon curIcon = blockBtn.getIcon();
            if (curIcon == enabledBlock) blockBtn.setIcon(disabledBlock);
            else blockBtn.setIcon(enabledBlock);
        }
    };
}
