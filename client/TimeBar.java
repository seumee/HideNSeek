import javax.swing.*;
import java.awt.*;
import java.awt.event.ActionListener;

public class TimeBar extends JLabel implements Runnable {
    int width = 1080, height = 5;
    int x, y;
    Color color = new Color(255, 0, 0);
    int sec;
    ActionListener timeoutSignal;
    float delay;
    public TimeBar(ActionListener timeoutSignal, int sec, int x, int y) {
        this.timeoutSignal = timeoutSignal;
        this.sec = sec;
        this.x = x;
        this.y = y;

        delay = (float)sec / (float)width;

        setBackground(color);
        setOpaque(true);
        setBounds(x, y, width, height*2);
    }

    @Override
    public void run() {
        while (true) {
            try {
                Thread.sleep((long)(delay*1000));
            } catch (Exception e) {}
            if (getWidth() > 0) {
                width -= 1;
                setBounds(x, y, width, height);
            } else {
                timeoutSignal.actionPerformed(null);
                break;
            }
        }
    }
}
