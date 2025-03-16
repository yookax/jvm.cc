package thread;

public class DaemonTest {
    
    public static void main(String[] args) {
        Thread mainThread = Thread.currentThread();
        System.out.println(!mainThread.isDaemon() ? "passed":"failed");

        Thread newThread = new Thread();
        System.out.println(!newThread.isDaemon() ? "passed":"failed");

        newThread.setDaemon(true);
        System.out.println(newThread.isDaemon() ? "passed":"failed");
    }
}
