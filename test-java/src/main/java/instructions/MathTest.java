package instructions;

import java.util.ArrayList;

public class MathTest {

    private static ArrayList<Boolean> list = new ArrayList<>();

    private static void testShift() {
        list.add((-2 << 1) == -4); // -4
        list.add((Integer.MAX_VALUE << 1) == -2); // -2

        list.add((4 >> 1) == 2); // 2
        list.add((-4 >> 1) == -2); // -2
        list.add((-4 >>> 1 == 2147483646)); // 2147483646（Integer.MAX_VALUE - 1）

        list.add((2 >> 1) == 1); // 1
        list.add((-2 >> 1) == -1); // -1
        list.add((-2 >>> 1) == 2147483647); // 2147483647（Integer.MAX_VALUE）

        list.add((1 >> 1) == 0); // 0
        list.add((-1 >> 1 == -1)); // -1
        list.add((-1 >>> 1) == 2147483647); // 2147483647（Integer.MAX_VALUE）

        list.add((12345 >> 13) == 1); // 1
        list.add((-12345 >> 13) == -2); // -2
        list.add((-12345 >>> 13) == 524286); // 524286

        long v = 12345678987654321L;
        list.add((v >> 13) == 1507040892047L); // 1507040892047
        list.add((-v >> 13) == -1507040892048L); // -1507040892048
        list.add((-v >>> 13) == 2250292772793200L); // 2250292772793200
    }

    private static void testRem() {
        int i = 3;
        int j = 2;
        list.add(i%j == 1); // irem

        float f1 = 23.56f;
        float f2 = 11.555f;
        list.add(Float.compare(f1%f2, 0.44999886F) == 0); // frem 0.44999886

        double d1 = 23444555454.554667776;
        double d2 = 4353511.555699655;
        list.add(Double.compare(d1%d2, 895727.1120270332) == 0); // drem 895727.1120270332
    }

    private static void testLogic() {
        long i = -776565656565677519L;
        long j = 457688856546456422L;
        list.add((i & j) == 294987450898580000L);
        list.add((i | j) == -613864250917801097L);
        list.add((i ^ j) == -908851701816381097L);
    }

    private static void testIinc() {
        int i = 50;

        i += -100; // iinc
        list.add(i == -50);

        i += 1000; // wind iinc
        list.add(i == 950);
    }

    public static void main(String[] args) {
        MathTest.testShift();
        MathTest.testRem();
        MathTest.testLogic();
        MathTest.testIinc();

        for (var b: list) {
            if (!b) {
                System.out.println("Failed");
                return;
            }
        }
        System.out.println("Passed");
    }
}
