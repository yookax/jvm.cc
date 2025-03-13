package thread;

public class InterruptFlagTest {
    
    public static void main(String[] args) {
        Thread t = Thread.currentThread();
        if (!t.isInterrupted())
            System.out.println("Pass");

        t.interrupt();
        if (t.isInterrupted())
            System.out.println("Pass");
        if (t.isInterrupted())
            System.out.println("Pass");

        if (Thread.interrupted())
            System.out.println("Pass");
        if (!Thread.interrupted())
            System.out.println("Pass");
        if (!t.isInterrupted())
            System.out.println("Pass");
    }
}
