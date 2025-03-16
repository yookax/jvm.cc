package thread;

public class InterruptFlagTest {
    
    public static void main(String[] args) {
        Thread t = Thread.currentThread();
        if (!t.isInterrupted())
            System.out.println("passed");

        t.interrupt();
        if (t.isInterrupted())
            System.out.println("passed");
        if (t.isInterrupted())
            System.out.println("passed");

        if (Thread.interrupted())
            System.out.println("passed");
        if (!Thread.interrupted())
            System.out.println("passed");
        if (!t.isInterrupted())
            System.out.println("passed");
    }
}
