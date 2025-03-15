package reflect;

import java.lang.reflect.Array;

public class ArrayGetTest {

    public static void main(String[] args) {
        var b1 = getNullArray();
        var b2 = getNonArray();
        var b3 = getArrayBadIndex();
        var b4 = getObjectArray();
        var b5 = getPrimitiveArray();
        System.out.println(b1 && b2 && b3 && b4 && b5 ? "passed" : "failed");
    }

    public static boolean getNullArray() {
        try {
            Object x = null;
            Array.get(x, 3);
        } catch (NullPointerException e) {
            return true;
        }
        return false;
    }

    public static boolean getNonArray() {
        try {
            String str = "abc";
            Array.get(str, 1);
        } catch (IllegalArgumentException e) {
            return true;
        }
        return false;
    }

    public static boolean getArrayBadIndex() {
        try {
            int[] arr = {1, 2, 3};
            Array.get(arr, -1);
        } catch (ArrayIndexOutOfBoundsException e) {
            return true;
        }
        return false;
    }

    public static boolean getObjectArray() {
        String[] arr = {"a", "b", "c"};
        return "c" == Array.get(arr, 2);
    }

    public static boolean getPrimitiveArray() {
        boolean b1 = ((Boolean) Array.get(new boolean[]{true}, 0));
        boolean b2 = ((byte) 2 == (Byte) Array.get(new byte[]{2}, 0));
        boolean b3 = ('a' == (Character) Array.get(new char[]{'a'}, 0));
        boolean b4 = ((short) 2 == (Short) Array.get(new short[]{2}, 0));
        boolean b5 = (2 == (Integer) Array.get(new int[]{2}, 0));
        boolean b6 = (2L == (Long) Array.get(new long[]{2}, 0));
        boolean b7 = (3.14f == (Float) Array.get(new float[]{3.14f}, 0));
        boolean b8 = (2.71 == (Double) Array.get(new double[]{2.71}, 0));
        return b1 && b2 && b3 && b4 && b5 && b6 && b7 && b8;
    }

}
