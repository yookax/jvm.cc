package object;

public class WrapperTest {

    public static boolean testDouble() {
        boolean b1 = (4614253070214989087L == Double.doubleToRawLongBits(3.14));
        boolean b2 = (3.14 == Double.longBitsToDouble(4614253070214989087L));
        return b1 && b2;
    }

    public static boolean testFloat() {
        boolean b1 = (1076754509 == Float.floatToRawIntBits(2.71828f));
        boolean b2 = (2.71828f == Float.intBitsToFloat(1076754509));
        return b1 && b2;
    }

    // Cache [-128, 127]
    public static boolean integerCache() {
        Integer a = -128;
        Integer b = -128;
        boolean x1 = (a == b);

        Integer a1 = 0;
        Integer b1 = 0;
        boolean x2 = (a1 == b1);

        Integer a2 = 127;
        Integer b2 = 127;
        boolean x3 = (a2 == b2);

        Integer c = -129;
        Integer d = -129;
        boolean x4 = (c != d);

        Integer c1 = 128;
        Integer d1 = 128;
        boolean x5 = (c1 != d1);
        return x1 && x2 && x3 && x4 && x5;
    }

    public static void main(String[] args) {
        System.out.println(testDouble() && testFloat() && integerCache());
    }
}
