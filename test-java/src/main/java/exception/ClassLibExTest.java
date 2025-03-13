package exception;

/**
 * Status: Fail
 */
public class ClassLibExTest {
    
    public static void main(String[] args) {
        threadSleep();
        threadSleep2();
    }

    public static void threadSleep() {
        try {
            Thread.sleep(-1);
        } catch (IllegalArgumentException | InterruptedException e) {
            e.printStackTrace();
        }
    }

    public static void threadSleep2() {
        Thread.currentThread().interrupt();
        System.out.println(Thread.currentThread().isInterrupted()); // true
        try {
            Thread.sleep(1000);
        } catch (InterruptedException e) {
            System.out.println(Thread.currentThread().isInterrupted()); // false
            e.printStackTrace();
        }
    }
    
}
