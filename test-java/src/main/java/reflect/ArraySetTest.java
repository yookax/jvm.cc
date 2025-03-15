package reflect;

import java.lang.reflect.Array;

public class ArraySetTest {
    public static void main(String[] args) {
        System.out.println(
                setNullArray()
                && setNonArray()
                && setArrayTypeMismatch()
                && setArrayBadIndex()
                && setObjectArray()
                && setPrimitiveArray()
                && setNullValue()
                && setWrongTypeValue()
                        ? "passed" : "failed"
        );
    }

    public static boolean setNullArray() {
        try {
            Object x = null;
            Array.set(x, 3, "a");
        } catch (NullPointerException e) {
            return true;
        }
        return false;
    }

    public static boolean setNonArray() {
        try {
            String str = "abc";
            Array.set(str, 1, "a");
        } catch (IllegalArgumentException e) {
            return true;
        }
        return false;
    }

    public static boolean setArrayTypeMismatch() {
        try {
            int[] arr = {1, 2, 3};
            Array.set(arr, 0, "beyond");
        } catch (IllegalArgumentException e) {
            return true;
        }
        return false;
    }

    public static boolean setArrayBadIndex() {
        try {
            int[] arr = {1, 2, 3};
            Array.set(arr, -1, 4);
        } catch (ArrayIndexOutOfBoundsException e) {
            return true;
        }
        return false;
    }

    public static boolean setObjectArray() {
        String[] arr = {"beyond"};
        Array.set(arr, 0, "5457");
        return ("5457" == Array.get(arr, 0));
    }

    public static boolean setPrimitiveArray() {
        Array.set(new boolean[]{true}, 0, false);
        Array.set(new byte[]{2}, 0, (byte) 3);
        Array.set(new char[]{'a'}, 0, 'b');
        Array.set(new short[]{2}, 0, (short) 3);
        Array.set(new int[]{2}, 0, 3);
        Array.set(new long[]{2}, 0, 3L);
        Array.set(new float[]{3.14f}, 0, 2.71f);
        Array.set(new double[]{2.71}, 0, 3.14);

        int[] arr = {5, 4, 5, 7};
        Array.set(arr, 0, 0);
        return (0 == (Integer) Array.get(arr, 0));
    }

    public static boolean setNullValue() {
        try {
            int[] arr = {1, 2, 3};
            Array.set(arr, 0, null);
        } catch (IllegalArgumentException e) {
            // do nothing
        }

        Object[] arr = {1, 2, 3};
        Array.set(arr, 0, null); // OK
        return true;
    }

    public static boolean setWrongTypeValue() {
        try {
            char[] aaa = { 'c' };
            Array.set(aaa, 0, new String("x"));
        } catch (IllegalArgumentException e) {
            return true;
        }
        return false;
    }
}
