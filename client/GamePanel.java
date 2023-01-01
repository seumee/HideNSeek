import javax.swing.*;
import javax.swing.Timer;
import java.awt.*;
import java.awt.event.*;
import java.util.*;

//import java.util.Random;
public class GamePanel extends JPanel implements ActionListener {
    /*----------------맵, 유닛 크기 설정----------------*/
    static final int SCREEN_WIDTH = 900;    //가로크기
    static final int SCREEN_HEIGHT = 960;   //세로크기
    static final int UNIT_SIZE = 30;        //유닛크기
    static final int GAME_UNITS = (SCREEN_WIDTH * SCREEN_HEIGHT) / UNIT_SIZE;   //유닛 수
    /*------------------------------------------------*/

    /*------------------게임 맵, 포탈-------------------*/
    int map[][];
    HashMap<Integer, ArrayList<Integer>> portalMap = new HashMap<>();
    /*------------------------------------------------*/

    /*-------------------좌표, 방향--------------------*/
    int curX = 15;
    int curY = 15;
    char direction = 'R';
    char lastLR = 'R';
    int otherX = 5;
    int otherY = 5;
    /*------------------------------------------------*/

    /*------------------대기방 설정값-------------------*/
    int runnerNum;                      //남은 runner 수
    static final int DELAY = 180;        //게임 속도     //조절 가능하도록 변경.
    /*------------------------------------------------*/

    /*------------------캐릭터 이미지-------------------*/
    ImageIcon myIcon = new ImageIcon(GamePanel.class.getResource("characters/RED.png"));
    Image myImg = myIcon.getImage();
    ImageIcon otherIcon = new ImageIcon(GamePanel.class.getResource("characters/GREEN.png"));         //tmp
    Image otherImg = otherIcon.getImage();                                                                     //tmp

    //Image scaledImg = img.getScaledInstance((SCREEN_WIDTH*SCREEN_HEIGHT)/UNIT_SIZE, (SCREEN_WIDTH*SCREEN_HEIGHT)/UNIT_SIZE, Image.SCALE_SMOOTH);
    /*------------------------------------------------*/

    boolean running = false;            //게임 진행 상태
    Timer timer;                        //timer: 일정 주기마다 이벤트 발생
    int result;
    TreeSet<Integer> pressedKey = new TreeSet<>();


    GamePanel(int map[][]) {
        this.map = map;
        this.setPreferredSize(new Dimension(SCREEN_WIDTH, SCREEN_HEIGHT));
        this.setLayout(null);
        this.setBounds(0,0,SCREEN_WIDTH,SCREEN_HEIGHT);
        this.setBackground(new Color(211, 211, 211));
        this.setFocusable(true);
        this.addKeyListener(new MyKeyAdapter());

        for (int i = 0; i < SCREEN_WIDTH/UNIT_SIZE; i++) {
            for (int j = 0; j < SCREEN_HEIGHT/UNIT_SIZE; j++) {
                int curVal = map[j][i];
                if(curVal  > 50 && curVal < 100) {
                    portalMap.put(curVal, new ArrayList<>());
                    portalMap.get(curVal).add(i);
                    portalMap.get(curVal).add(j);
                }
            }
        }
        startGame();
    }

    public void startGame() {       //게임시작
        //플레이어 역할 배정
        //대기방 조절값 적용
        /*게임 시작 (running = true)*/
        running = true;
        timer = new Timer(DELAY, this);
        timer.start();
    }

    public void paintComponent(Graphics g) {        //그리기 메소드
        super.paintComponent(g);
        drawMap(g);
        drawMyCharacter(g);
        drawOtherCharacter(g);
    }

    public void drawMyCharacter(Graphics g) {      //캐릭터 표시
        if(running) {
            if(lastLR == 'L') {
                g.drawImage(myImg, (curX+1)*UNIT_SIZE, curY*UNIT_SIZE, (0-UNIT_SIZE), UNIT_SIZE, this);
            }
            if(lastLR == 'R') {
                g.drawImage(myImg, curX*UNIT_SIZE, curY*UNIT_SIZE, UNIT_SIZE, UNIT_SIZE, this);
            }
        }
        else gameOver(g);
    }

    public void drawOtherCharacter(Graphics g) {
        if(running) {
            g.drawImage(otherImg, (otherX+1)*UNIT_SIZE, otherY*UNIT_SIZE, (0-UNIT_SIZE), UNIT_SIZE, this);
        }
    }

    public void drawMap(Graphics g) {       //맵 표시
        if(running) {
            for (int i = 0; i < SCREEN_WIDTH / UNIT_SIZE; i++) {
                for (int j = 0; j < SCREEN_HEIGHT / UNIT_SIZE; j++) {
                    if (map[j][i] == 1) {
                        g.setColor(new Color(0, 0, 0));
                        g.fillRect(i * UNIT_SIZE, j * UNIT_SIZE, UNIT_SIZE, UNIT_SIZE);
                    }
                }
            }
            //Tagger 스킬 감지. block 여부에 따라 장애물 설치
        }
    }

    public void move() {
        if(!pressedKey.isEmpty()) {
            switch (direction) {
                case 'U':
                    result = checkIsWall(curX, curY-1);
                    if(result == 1) curY--;
                    else if(result > 50) {
                        curX = portalMap.get(result).get(0);
                        curY = portalMap.get(result).get(1);
                    }
                    break;
                case 'D':
                    result = checkIsWall(curX, curY+1);
                    if(result == 1) curY++;
                    else if(result > 50) {
                        curX = portalMap.get(result).get(0);
                        curY = portalMap.get(result).get(1);
                    }
                    break;
                case 'L':
                    lastLR = 'L';
                    result = checkIsWall(curX-1, curY);
                    if(result == 1) curX--;
                    else if(result > 50) {
                        curX = portalMap.get(result).get(0);
                        curY = portalMap.get(result).get(1);
                    }
                    break;
                case 'R':
                    lastLR = 'R';
                    result = checkIsWall(curX+1, curY);
                    if(result == 1) curX++;
                    else if(result > 50) {
                        curX = portalMap.get(result).get(0);
                        curY = portalMap.get(result).get(1);
                    }
                    break;
            }
            if(curX == 0) curX = curX + 1;
            else if(curX == (SCREEN_WIDTH/UNIT_SIZE - 1)) curX = curX - 1;
            else if(curY == 0) curY = curY + 1;
            else if(curY == SCREEN_HEIGHT/UNIT_SIZE) curY = curY - 1;

            canCatch();
        }
    }
    public int checkIsWall(int X, int Y) {
        if(map[Y][X] == 1) {
            return 0;
        } else if (map[Y][X] > 50) {
            return 150 - map[Y][X];
        }
        return 1;
    }

    public void countRunners() {
        if(runnerNum == 0) timer.stop();
    }

    public void skillCatch() {

    }

    public void canCatch() {

    }


    public void gameOver(Graphics g) {

    }

    @Override
    public void actionPerformed(ActionEvent e) {        //타이머 이벤트마다 호출. move/countRunners
        if (running) {
            move();
        }
        repaint();
    }

    public class MyKeyAdapter extends KeyAdapter {
        @Override
        public void keyPressed(KeyEvent e) {
            int whichKey = e.getKeyCode();
            pressedKey.add(whichKey);
            switch (whichKey) {
                case KeyEvent.VK_LEFT:
                    direction = 'L';
                    break;
                case KeyEvent.VK_RIGHT:
                    direction = 'R';
                    break;
                case KeyEvent.VK_UP:
                    direction = 'U';
                    break;
                case KeyEvent.VK_DOWN:
                    direction = 'D';
                    break;
            }
        }

        @Override
        public void keyReleased(KeyEvent e) {
            pressedKey.remove(e.getKeyCode());
            if(pressedKey.size() == 1) {
                switch (pressedKey.first()) {
                    case KeyEvent.VK_LEFT:
                        direction = 'L';
                        break;
                    case KeyEvent.VK_RIGHT:
                        direction = 'R';
                        break;
                    case KeyEvent.VK_UP:
                        direction = 'U';
                        break;
                    case KeyEvent.VK_DOWN:
                        direction = 'D';
                        break;
                }
            }
        }
    }
}
