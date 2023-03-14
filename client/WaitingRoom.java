import javax.swing.*;
import javax.swing.Timer;
import java.awt.*;
import java.awt.event.*;
import java.util.*;

public class WaitingRoom extends JPanel implements ActionListener {
    /*----------------SIZE setting----------------*/
    static final int SCREEN_WIDTH = 450;    //가로크기
    static final int SCREEN_HEIGHT = 480;   //세로크기
    static final int UNIT_SIZE = 30;        //유닛크기
    /*------------------------------------------------*/

    /*-----------------Map & Portal------------------*/
    byte map[][] = new byte[16][15];
    /*------------------------------------------------*/

    /*-------------Position & Direction--------------*/
    char direction = 'R';
    char lastLR = 'R';
    /*------------------------------------------------*/

    /*-----------------Game Options------------------*/
    static final int DELAY = 180;        //게임 속도     //조절 가능하도록 변경.
    /*------------------------------------------------*/

    /*----------------Character Image-----------------*/
    ImageIcon[] Icons = new ImageIcon[8];
    Image[] Img = new Image[8];
    Colors colors = Colors.BLACK;

    /*------------------------------------------------*/


    Timer timer;                        //timer: 일정 주기마다 이벤트 발생
    int tickCalc = 0;
    static final int TICKCOUNT = 3 * 1000;
    TreeSet<Integer> pressedKey = new TreeSet<>();

    SocketHandler handler;
    Values.WriteValues writeValues;
    Values.ReadValues readValues;
    Values.StaticValues staticValues;
    ActionListener startListener;
    ActionListener optionListener;

    WaitingRoom(ActionListener startListener, ActionListener optionListener, SocketHandler handler, Values values) {
        setMap();
        this.startListener = startListener;
        this.optionListener = optionListener;
        this.writeValues = values.writeValues;
        this.readValues = values.readValues;
        this.staticValues = values.staticValues;
        this.handler = handler;

        staticValues.isalive = true;

        writeValues.x = staticValues.playerCor[staticValues.colorIdx][0];
        writeValues.y = staticValues.playerCor[staticValues.colorIdx][1];

        setPreferredSize(new Dimension(SCREEN_WIDTH, SCREEN_HEIGHT));
        setLayout(null);
        setBounds(0,0,SCREEN_WIDTH,SCREEN_HEIGHT);
        setBackground(new Color(211, 211, 211));
        addKeyListener(new MyKeyAdapter());
        setFocusable(true);

        setIcons();

        init();
    }

    private void setIcons() {
        for (int i = 0; i < 8; i++) {
            Icons[i] = new ImageIcon(InGame.class.getResource("characters/"+colors.Colors(i)+".png"));
            Img[i] = Icons[i].getImage();
        }
    }
    private void setMap() {
        for(int i = 0; i < 16; i++) {
            for(int j = 0; j < 15; j++) {
                map[i][j] = 1;
            }
        }
        for(int i = 1; i < 15; i++) {
            for(int j = 1; j < 14; j++) {
                map[i][j] = 0;
            }
        }
    }

    public void init() {       //게임시작
        timer = new Timer(DELAY, this);
        timer.start();
    }

    public void paintComponent(Graphics g) {        //그리기 메소드
        super.paintComponent(g);
        drawMap(g);
        drawCharacter(g);
    }

    public void drawCharacter(Graphics g) {
        for (int i = 0; i < 8; i++) {
            if (staticValues.playerCor[i][0] != 0) {
                g.drawImage(Img[i], (staticValues.playerCor[i][0]+1)*UNIT_SIZE, staticValues.playerCor[i][1]*UNIT_SIZE, (0-UNIT_SIZE), UNIT_SIZE, this);
            }
        }
    }

    public void drawMap(Graphics g) {
        for (int i = 0; i < SCREEN_WIDTH / UNIT_SIZE; i++) {
            for (int j = 0; j < SCREEN_HEIGHT / UNIT_SIZE; j++) {
                if (map[j][i] == 1) {
                    g.setColor(new Color(0, 0, 0));
                    g.fillRect(i * UNIT_SIZE, j * UNIT_SIZE, UNIT_SIZE, UNIT_SIZE);
                }
            }
        }
    }

    public void move() {
        if(!pressedKey.isEmpty()) {
            int[] me = staticValues.playerCor[staticValues.colorIdx];
            switch (direction) {
                case 'U':
                    if(checkCanMove(me[0], me[1] - 1)) me[1]--;
                    break;
                case 'D':
                    if(checkCanMove(me[0], me[1] +1)) me[1]++;
                    break;
                case 'L':
                    lastLR = 'L';
                    if(checkCanMove(me[0] -1, me[1])) me[0]--;
                    break;
                case 'R':
                    lastLR = 'R';
                    if(checkCanMove(me[0] +1, me[1])) me[0]++;
                    break;
                case 'S':
                    return;
            }
            if(me[0] == 0) me[0] = me[0] + 1;
            else if(me[0] == (SCREEN_WIDTH/UNIT_SIZE - 1)) me[0] = me[0] - 1;
            else if(me[1] == 0) me[1] = me[1] + 1;
            else if(me[1] == SCREEN_HEIGHT/UNIT_SIZE) me[1] = me[1] - 1;

            writeValues.msgType = MsgType.MSG_POSITION;
            writeValues.x = me[0];
            writeValues.y = me[1];

            writeValues.writeListener.actionPerformed(null);
            tickCalc = 0;
        }
    }
    public boolean checkCanMove(int X, int Y) {
        if(map[Y][X] == 1) return false;
        else return true;
    }

    @Override
    public void actionPerformed(ActionEvent e) {        //타이머 이벤트마다 호출. move/countRunners
        tickCalc += DELAY;
        if(tickCalc > TICKCOUNT) {
            writeValues.msgType = MsgType.MSG_POSITION;
            writeValues.writeListener.actionPerformed(null);
            tickCalc = 0;
        }
        move();
        repaint();
    }

    public class MyKeyAdapter extends KeyAdapter {
        @Override
        public void keyPressed(KeyEvent e) {
            int whichKey = e.getKeyCode();
            pressedKey.add(whichKey);
            switch (whichKey) {
                case KeyEvent.VK_A:
                    direction = 'L';
                    break;
                case KeyEvent.VK_D:
                    direction = 'R';
                    break;
                case KeyEvent.VK_W:
                    direction = 'U';
                    break;
                case KeyEvent.VK_S:
                    direction = 'D';
                    break;
                case KeyEvent.VK_SPACE:
                    direction = 'S';
                    if(staticValues.colorIdx == staticValues.adminColorIdx) {
                        startListener.actionPerformed(null);    //start Game}
                    }
                    break;
                case KeyEvent.VK_O:
                    direction = 'S';
                    optionListener.actionPerformed(null);    //show Option
                    break;
                default:
                    direction = 'S';    //stop
                    break;
            }
        }

        @Override
        public void keyReleased(KeyEvent e) {
            pressedKey.remove(e.getKeyCode());
            if(pressedKey.size() == 1) {
                switch (pressedKey.first()) {
                    case KeyEvent.VK_A:
                        direction = 'L';
                        break;
                    case KeyEvent.VK_D:
                        direction = 'R';
                        break;
                    case KeyEvent.VK_W:
                        direction = 'U';
                        break;
                    case KeyEvent.VK_S:
                        direction = 'D';
                        break;
                    default:
                        direction = 'S';    //stop
                        break;
                }
            }
        }
    }
}
