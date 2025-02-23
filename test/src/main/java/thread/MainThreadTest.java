package thread;

public class MainThreadTest {
    
    public static void main(String[] args) {
        Thread mainThread = Thread.currentThread();
        System.out.println("name: " + mainThread.getName()); // main
        System.out.println("isAlive: " + mainThread.isAlive()); // true
        System.out.println("isDaemon: " + mainThread.isDaemon()); // false
        System.out.println("ThreadGroup: " + mainThread.getThreadGroup()); // java.lang.ThreadGroup[name=main,maxpri=10]
    }
}
