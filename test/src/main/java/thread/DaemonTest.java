package thread;

/**
 * Status: Pass
 */
public class DaemonTest {
    
    public static void main(String[] args) {
        Thread mainThread = Thread.currentThread();
        System.out.println(!mainThread.isDaemon() ? "Pass":"Fail");

        Thread newThread = new Thread();
        System.out.println(!newThread.isDaemon() ? "Pass":"Fail");

        newThread.setDaemon(true);
        System.out.println(newThread.isDaemon() ? "Pass":"Fail");
    }
}
