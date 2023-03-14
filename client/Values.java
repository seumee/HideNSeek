import java.awt.event.ActionListener;

public class Values {
    static final int ROLE_TAGGER = 1;
    static final int ROLE_RUNNER = 0;
    static final int DEAD_RUNNER = -1;
    static final int STATUS_PAUSE = 0;
    static final int STATUS_RESUME = 1;
    static final int  RSP_VALUE_FAIL = -1;
    static final int RSP_VALUE_SUCC = 0;
    static final int RSP_QRY_FAIL = -1;
    static final int RSP_QRY_NONEXT = 0;
    static final int  RSP_QRY_ISNEXT = 1;
    static final int  RSP_VALUE_FULL_ROOM = -2;
    static final int  RSP_VALUE_RUNNING_ROOM = -3;
    static final int PAGE = 10;

    public static class WriteValues {
        ActionListener writeListener;
        MsgType msgType;
        String roomname;
        int gameStatus;
        int target;
        int page;
        int x, y;
        int blockX, blockY;
    }
    WriteValues writeValues = new WriteValues();

    public static class ReadValues {
        ActionListener readListener;
        ActionListener[] listeners = new ActionListener[2];
        ActionListener startListener;
        ActionListener adminListener;
        ActionListener gameEndListener;
        class roomList {
            int[] roomId = new int[PAGE];
            String[] roomName = new String[PAGE];
            int[] userCount = new int[PAGE];
        }
        int msgType;
        int result;
        int isnext;
        String nickname;
        roomList roomlist = new roomList();
        int tagger;
        int target;
        int page;
        int isalive;
        int blockX, blockY;
        int totalTime;
    }
    ReadValues readValues = new ReadValues();

    public static class gameOptions {
        int numberOfTagger;
        int gameSpeed;
        int sight_disadvantage;
        int taggerSpeed;
        int catch_cooldown;
        int block_cooldown;
    }
    public void setDefaultOptions() {
        staticValues.options.numberOfTagger = 2;
        staticValues.options.gameSpeed = 100;
        staticValues.options.sight_disadvantage = 30;
        staticValues.options.taggerSpeed = 120;
        staticValues.options.catch_cooldown = 10;
        staticValues.options.block_cooldown = 20;
    }

    public static class StaticValues {
        static final int BLOCK_DURATION = 10;
        gameOptions options = new gameOptions();
        int roles[] = new int[8];
        int taggerScore[] = new int[8];
        boolean amITagger;
        boolean isalive;
        int roomId;
        String nickname;
        String roomname;
        int colorIdx = -1;
        int[][] playerCor = new int[8][2];
        int adminColorIdx = 0;
        int userCount;
        int[] user;
        int roomState;
        ActionListener blockTimerListener;
        boolean canCatch;
    }
    StaticValues staticValues = new StaticValues();
}
