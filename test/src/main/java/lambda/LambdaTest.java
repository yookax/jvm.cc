package lambda;

public class LambdaTest {
    public static void main(String[] args) {
        Runnable r = () -> System.out.println("Simple lambda.");
        r.run();
    }
}
