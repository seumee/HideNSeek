import javax.swing.*;
import javax.swing.Timer;
import java.awt.*;
import java.awt.event.*;
import java.util.*;

//import java.util.Random;
public class InGame extends JPanel implements ActionListener {
    /*-------------Read&Write values--------------*/
    Values values;
    Values.WriteValues writeValues;
    Values.ReadValues readValues;
    Values.StaticValues staticValues;

    /*----------------SIZE setting----------------*/
    static final int SCREEN_WIDTH = 900;    //가로크기
    static final int SCREEN_HEIGHT = 960;   //세로크기
    static final int UNIT_SIZE = 30;        //유닛크기
    static final int GAME_UNITS = (SCREEN_WIDTH * SCREEN_HEIGHT) / UNIT_SIZE;   //유닛 수
    /*------------------------------------------------*/

    /*-----------------Map & Portal------------------*/
    static final int ROAD = 0;
    static final int WALL = 1;
    static final int BLOCK = 2;
    byte map[][];
    HashMap<Integer, ArrayList<Integer>> portalMap = new HashMap<>();
    /*------------------------------------------------*/

    /*-------------Position & Direction--------------*/
    char direction = 'R';
    char lastLR = 'R';
    /*------------------------------------------------*/

    /*-----------------Game Options------------------*/
    int runnerNum;                      //남은 runner 수
    int DELAY;        //게임 속도     //조절 가능하도록 변경.
    /*------------------------------------------------*/

    /*----------------Character Image-----------------*/
    ImageIcon[] Icons = new ImageIcon[8];
    Image[] Img = new Image[8];
    ImageIcon tombIcon = new ImageIcon(InGame.class.getResource("icons/tomb.png"));
    Colors colors = Colors.BLACK;
    /*------------------------------------------------*/

    /*----------------Sight Fog Image-----------------*/
    int sightSize;
    ImageIcon sightIcon = new ImageIcon(InGame.class.getResource("Image/sight.png"));
    Image scaledSight;
    /*------------------------------------------------*/
    private Image getScaledImg(ImageIcon original, int w, int h) {
        Image tmpImg = original.getImage();
        return tmpImg.getScaledInstance(w, h, Image.SCALE_SMOOTH);
    }


    boolean running = false;            //게임 진행 상태
    Timer timer;                        //timer: 일정 주기마다 이벤트 발생
    int result;
    TreeSet<Integer> pressedKey = new TreeSet<>();

    SocketHandler handler;
    long curTime;
    long blockTimeCheck = 0;
    long catchTimeCheck = 0;
    boolean isCatchInCooldown = false;
    static final int BLOCK_LISTENER = 0;
    static final int CATCH_LISTENER = 1;
    ActionListener catchSignal;
    ActionListener checkCanCatch;

    KeyListener keyListener = new MyKeyAdapter();

    InGame(byte map[][], ActionListener checkCanCatch, ActionListener catchSignal, SocketHandler handler, Values values) {        this.values = values;
        this.writeValues = values.writeValues;
        this.readValues = values.readValues;
        this.staticValues = values.staticValues;
        this.handler = handler;
        this.map = map;
        this.catchSignal = catchSignal;
        this.setPreferredSize(new Dimension(SCREEN_WIDTH, SCREEN_HEIGHT));
        this.setLayout(null);
        this.setBounds(0,0,SCREEN_WIDTH,SCREEN_HEIGHT);
        this.setBackground(new Color(211, 211, 211));
        this.setFocusable(true);
        this.addKeyListener(keyListener);
        this.addMouseListener(new WallListener());

        this.checkCanCatch = checkCanCatch;
        readValues.listeners[BLOCK_LISTENER] = BlockListener;
        readValues.listeners[CATCH_LISTENER] = CatchListener;

        runnerNum = staticValues.userCount-staticValues.options.numberOfTagger;

        sightSize = 180*staticValues.options.sight_disadvantage;
        scaledSight = getScaledImg(sightIcon, sightSize, sightSize);

        setInitPosition();
        setPortals(map);
        setIcons();
        startGame();
    }

    private void setInitPosition() {
        for(int i = 0; i < 8; i++) {
            if(staticValues.playerCor[i][0] != 0) {
                if(staticValues.roles[i] == Values.ROLE_TAGGER) {
                    staticValues.playerCor[i][0] = 15;
                    staticValues.playerCor[i][1] = 15;
                } else {
                    staticValues.playerCor[i][0] = 15;
                    staticValues.playerCor[i][1] = 12;
                }
            }
        }
    }

    private void setPortals(byte[][] map) {
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
    }

    private void setIcons() {
        for (int i = 0; i < 8; i++) {
            Icons[i] = new ImageIcon(InGame.class.getResource("characters/"+colors.Colors(i)+".png"));
            Img[i] = Icons[i].getImage();
        }
    }

    private void procDead(int colorIdx) {
        staticValues.roles[colorIdx] = values.DEAD_RUNNER;
        Img[colorIdx] = tombIcon.getImage();
    }

    public void startGame() {       //게임시작
        //플레이어 역할 배정
        //대기방 조절값 적용
        running = true;
        DELAY = (int)(180*(100/(float)staticValues.options.gameSpeed));
        timer = new Timer(DELAY, this);
        timer.start();
    }

    public void paintComponent(Graphics g) {
        super.paintComponent(g);
        drawCharacter(g);
        if(staticValues.amITagger) viewRange(g);
        drawMap(g);
    }

    public void drawCharacter(Graphics g) {
        if (running) {
            for (int i = 0; i < 8; i++) {
                if (staticValues.playerCor[i][0] != 0) {
                    g.drawImage(Img[i], (staticValues.playerCor[i][0])*UNIT_SIZE, staticValues.playerCor[i][1]*UNIT_SIZE, UNIT_SIZE, UNIT_SIZE, this);
                }
            }
        } else {
            gameOver(g);
        }
    }

    public void drawMap(Graphics g) {       //맵 표시
        if(running) {
            for (int i = 0; i < SCREEN_WIDTH / UNIT_SIZE; i++) {
                for (int j = 0; j < SCREEN_HEIGHT / UNIT_SIZE; j++) {
                    if (map[j][i] == WALL) {
                        g.setColor(new Color(0, 0, 0));
                        g.fillRect(i * UNIT_SIZE, j * UNIT_SIZE, UNIT_SIZE, UNIT_SIZE);
                    } else if (map[j][i] == BLOCK) {
                        g.setColor(new Color(150, 0, 0));
                        g.fillRect(i * UNIT_SIZE, j * UNIT_SIZE, UNIT_SIZE, UNIT_SIZE);
                    }
                }
            }
        }
    }

    public void viewRange(Graphics g) {
        if (running) {
            int[] me = staticValues.playerCor[staticValues.colorIdx];
            g.drawImage(scaledSight, (me[0] * UNIT_SIZE)-((sightSize-UNIT_SIZE)/2),(me[1] * UNIT_SIZE)-((sightSize-UNIT_SIZE)/2), this);
        }
    }

    public void move() {
        if(!pressedKey.isEmpty() && staticValues.isalive) {
            int[] me = staticValues.playerCor[staticValues.colorIdx];
            switch (direction) {
                case 'U':
                    result = checkIsWall(me[0], me[1] - 1);
                    if (result == 1) me[1]--;
                    else if (result > 50) {
                        me[0] = portalMap.get(result).get(0);
                        me[1] = portalMap.get(result).get(1);
                    }
                    break;
                case 'D':
                    result = checkIsWall(me[0], me[1] + 1);
                    if (result == 1) me[1]++;
                    else if (result > 50) {
                        me[0] = portalMap.get(result).get(0);
                        me[1] = portalMap.get(result).get(1);
                    }
                    break;
                case 'L':
                    lastLR = 'L';
                    result = checkIsWall(me[0] - 1, me[1]);
                    if (result == 1) me[0]--;
                    else if (result > 50) {
                        me[0] = portalMap.get(result).get(0);
                        me[1] = portalMap.get(result).get(1);
                    }
                    break;
                case 'R':
                    lastLR = 'R';
                    result = checkIsWall(me[0] + 1, me[1]);
                    if (result == 1) me[0]++;
                    else if (result > 50) {
                        me[0] = portalMap.get(result).get(0);
                        me[1] = portalMap.get(result).get(1);
                    }
                    break;
                case 'S':
                    return;
            }
            if (me[0] == 0) me[0] = me[0] + 1;
            else if (me[0] == (SCREEN_WIDTH / UNIT_SIZE - 1)) me[0] = me[0] - 1;
            else if (me[1] == 0) me[1] = me[1] + 1;
            else if (me[1] == SCREEN_HEIGHT / UNIT_SIZE) me[1] = me[1] - 1;

            writeValues.msgType = MsgType.MSG_POSITION;
            writeValues.x = me[0];
            writeValues.y = me[1];

            writeValues.writeListener.actionPerformed(null);
        }
    }
    public int checkIsWall(int X, int Y) {
        if(map[Y][X] == 1 || map[Y][X] == 2) return 0;
        else if (map[Y][X] > 50) return 150 - map[Y][X];
        return 1;
    }

    public void gameOver(Graphics g) {
        //점수 표시
        g.setColor(Color.RED);
        g.setFont(new Font("Ink Free", Font.BOLD, 40));
        FontMetrics metrics1 = getFontMetrics(g.getFont());
        g.drawString("Total Time: "+readValues.totalTime, (SCREEN_WIDTH - metrics1.stringWidth("Total Time: "+readValues.totalTime))/2, g.getFont().getSize());
        //게임오버 텍스트
        g.setColor(Color.RED);
        g.setFont(new Font("Ink Free", Font.BOLD, 75));
        FontMetrics metrics2 = getFontMetrics(g.getFont());
        g.drawString("Game Over", (SCREEN_WIDTH - metrics2.stringWidth("Game Over"))/2, SCREEN_HEIGHT/2);
        repaint();
        timer.stop();
    }

    @Override        //타이머 이벤트마다 호출.
    public void actionPerformed(ActionEvent e) {
        if (running) {
            move();
            repaint();
            curTime = System.currentTimeMillis();
            //Catch
            checkCatchCooldown();
            if(!isCatchInCooldown) {
                if(canCatch() != -1) staticValues.canCatch = true;
                else staticValues.canCatch = false;
            }
            checkCanCatch.actionPerformed(null);
            //Block
            if(checkBlockTick()) staticValues.blockTimerListener.actionPerformed(null);
            checkBlockDuration();
        }
    }


    /*-----------Block functions-----------*/
    public boolean checkBlockCooldown() {
        if(curTime - blockTimeCheck >= 1000*staticValues.options.block_cooldown) return true;
        return false;
    }
    public boolean checkBlockTick() {
        if(curTime - blockTimeCheck >= 1000*staticValues.options.block_cooldown
            && curTime - blockTimeCheck < 1000*staticValues.options.block_cooldown+DELAY) return true;
        return false;
    }
    public void checkBlockDuration() {
        if(curTime - blockTimeCheck >= 1000*staticValues.BLOCK_DURATION
                && curTime - blockTimeCheck < 1000*staticValues.BLOCK_DURATION+DELAY) {
            map[readValues.blockX][readValues.blockY] = ROAD;
            readValues.blockX = 0;
            readValues.blockY = 0;
        }
    }
    public class WallListener extends MouseAdapter {
        @Override
        public void mouseClicked(MouseEvent e) {
            if(staticValues.amITagger && checkBlockCooldown()) {            //for test. !amITagger -> amITagger로 변경.
                int mouseX = e.getY() / (SCREEN_WIDTH / UNIT_SIZE);
                int mouseY = e.getX() / (SCREEN_WIDTH / UNIT_SIZE);
                if(map[mouseX][mouseY] == ROAD) {
                    writeValues.blockX = mouseX;
                    writeValues.blockY = mouseY;
                    writeValues.msgType = MsgType.MSG_BLOCK;
                    writeValues.writeListener.actionPerformed(null);
                }
            }
        }
    }
    ActionListener BlockListener = new ActionListener() {
        @Override
        public void actionPerformed(ActionEvent e) {
            blockTimeCheck = System.currentTimeMillis();
            map[readValues.blockX][readValues.blockY] = BLOCK;
            staticValues.blockTimerListener.actionPerformed(null);
        }
    };

    /*-----------Catch functions-----------*/
    public void checkCatchCooldown() {
        if(curTime - catchTimeCheck >= 1000*staticValues.options.catch_cooldown) isCatchInCooldown = false;
    }
    ActionListener CatchListener = new ActionListener() {
        @Override
        public void actionPerformed(ActionEvent e) {
            staticValues.taggerScore[readValues.tagger] += 1;
            if(readValues.tagger == staticValues.colorIdx) {
                staticValues.canCatch = false;
                isCatchInCooldown = true;
                catchTimeCheck = curTime;
            }
            procDead(readValues.target);
        }
    };
    public int canCatch() {
        if(!isCatchInCooldown) {
            int[] me = staticValues.playerCor[staticValues.colorIdx];
            for (int i = 0; i < 8; i++) {
                if (staticValues.roles[i] == Values.ROLE_RUNNER &&
                        Math.abs(staticValues.playerCor[i][0] - me[0]) <= 1 && Math.abs(staticValues.playerCor[i][1] - me[1]) <= 1) {
                    return i;
                }
            }
        }
        return -1;
    }

    /*-------------Key Listener-------------*/
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
                    int whom = canCatch();
                    if (whom != -1) {
                        writeValues.target = whom;
                        catchSignal.actionPerformed(null);
                    }
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
