
public class Ackermann {

    public static void main(String[] args) {
        if (ackermann(3L, 5L) != 253) {
            System.out.println("failed");
        } else {
            System.out.println("passed");
        }
    }

    public static long ackermann(long m, long n) {
        if (m == 0) return n + 1;
        if (n == 0) return ackermann(m - 1, 1);
        return ackermann(m - 1, ackermann(m, n - 1));
    }
}
