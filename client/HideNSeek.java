import java.net.InetAddress;
import java.net.UnknownHostException;

public class HideNSeek {
    public static void main(String[] args) {
        try {
            //LocalHost -> 서버 ddns 도메인으로 변경 필요
            InetAddress ia = InetAddress.getLocalHost();
            String ip_str = ia.toString();
            String ip = ip_str.substring(ip_str.indexOf("/") + 1);
            new GameMap(ip, 2550);
        } catch (UnknownHostException e) {
            e.printStackTrace();
        }
    }
}
