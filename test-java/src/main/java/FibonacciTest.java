
public class FibonacciTest {

    public static void main(String[] args) {
        if (fib(20) != 6765) {
            System.out.println("failed");
        } else {
            System.out.println("passed");
        }
    }

    public static long fib(final int n) {
        if (n <= 2) {
            return (n > 0) ? 1 : 0;
        }
        return fib(n - 1) + fib(n - 2);
    }
}
