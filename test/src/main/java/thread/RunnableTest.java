package thread;

public class RunnableTest implements Runnable {
    
    public static void main(String[] args) throws InterruptedException {
        Thread t = new Thread(new RunnableTest());
        t.start();
        
        for (int i = 0; i < 100; i++) {
            System.out.println("main:" + i);
        }
        t.join();
    }

    @Override
    public void run() {
        for (int i = 0; i < 100; i++) {
            System.out.println("run:" + i);
        }
    }
    
}
