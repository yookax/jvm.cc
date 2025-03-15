package reflect;

import java.lang.reflect.Field;

public class FieldTest extends Thread {

    char a1, a2, a3, a4;
    static int x = 123;
    int y;

    public FieldTest(int y) { this.y = y; }
    
    public static void main(String[] args) throws Exception {
        Field fx = FieldTest.class.getDeclaredField("x");
        fx.setAccessible(true);
        if ((Integer) fx.get(null) != 123) {
            System.out.println("failed");
            return;
        }

        Field fy = FieldTest.class.getDeclaredField("y");
        fy.setAccessible(true);
        if ((Integer) fy.get(new FieldTest(456)) != 456) {
            System.out.println("failed");
            return;
        }

        System.out.println("passed");
    }
    
}
