package thread;

public class ThreadTest {
    public static void main(String[] args) throws InterruptedException {
        var r = new Runnable() {
            @Override
            public void run() {
                for (int i = 0; i < 10; i++) {
                    System.out.println("run:" + i);
                }
            }
        };

        var t = new Thread(r);
        System.out.println(t.isAlive());
        t.start();
        System.out.println(t.isAlive());
        System.out.println("main");
        //t.join();
    }
}
