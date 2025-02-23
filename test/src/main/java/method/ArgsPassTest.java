package method;

import java.util.Arrays;

public class ArgsPassTest {
    
    public static void main(String[] args) {
        String[] strArray = { "abc", "def", "xyz" };
        f(-100, 98765432101234L, 3.14f, 5452.71828, strArray);
        new ArgsPassTest().g(123, 98765432101234L, 3.14f, 5452.71828, strArray);
    }
    
    private static void f(int a, long b, float c, double d, Object e) {
        int x = a;
        long y = b;
        float z = c;
        double u = d;
        Object v = e;

        System.out.print("-100, 98765432101234, 3.14, 5452.71828, [abc, def, xyz] <---> ");
        System.out.printf("%d, %d, %.2f, %.5f, %s\n", x, y, z, u, Arrays.deepToString((Object[]) v));
    }
    
    private void g(int a, long b, float c, double d, Object e) {
        int x = a;
        long y = b;
        float z = c;
        double u = d;
        Object v = e;

        System.out.print("123, 98765432101234, 3.14, 5452.71828, [abc, def, xyz] <---> ");
        System.out.printf("%d, %d, %.2f, %.5f, %s\n---\n", x, y, z, u,  Arrays.deepToString((Object[]) v));
    }    
}
