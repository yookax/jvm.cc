package reflect;

import java.lang.reflect.Array;

public class ArrayLengthTest {
    public static void main(String[] args) {
        System.out.println(nullArrayLength());
        System.out.println(nonArrayLength());
        System.out.println(arrayLength());
    }

    public static boolean nullArrayLength() {
        try {
            Object x = null;
            Array.getLength(x);
        } catch (NullPointerException e) {
            return true;
        }
        return false;
    }

    public static boolean nonArrayLength() {
        try {
            Array.getLength("A");
        } catch (IllegalArgumentException e) {
            return true;
        }
        return false;
    }

    public static boolean arrayLength() {
        return (Array.getLength(new int[0]) == 0) && (Array.getLength(new int[3]) == 3);
    }
}
