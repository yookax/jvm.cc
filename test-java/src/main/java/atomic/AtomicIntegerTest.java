package atomic;

import java.util.concurrent.atomic.AtomicInteger;

public class AtomicIntegerTest {
    
    private static final AtomicInteger x = new AtomicInteger(0);
    private static final AtomicInteger y = new AtomicInteger(0);
    
    private static class Runner extends Thread {
        
        @Override
        public void run() {
            for (int i = 0; i < 100; i++) {
                sleepMS(10);
                x.getAndIncrement();
            }
            System.out.print(y.incrementAndGet() + ", ");
        }
        
    }
    
    public static void main(String[] args) {
//        for (int i = 0; i < 100; i++) {
//            new Runner().start();
//        }
//        while (y.get() != 100) {
//            sleepMS(100);
//        }
//        AtomicInteger y = new AtomicInteger(0);
//        System.out.println("\ny=" + y.get());
    }
    
    private static void sleepMS(long millis) {
        try {
            Thread.sleep(millis);
        } catch (InterruptedException ex) {
            ex.printStackTrace(System.err);
        }
    }
    
}
