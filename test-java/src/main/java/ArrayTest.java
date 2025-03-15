import java.lang.reflect.Array;
import java.util.Arrays;

public class ArrayTest {

    public static boolean newArray() {
        var o1 = (int[]) Array.newInstance(int.class, 15);
        boolean b1 = (o1.getClass() == int[].class);
        boolean b2 = (o1.length == 15);

        var o2 = (int[][]) Array.newInstance(int[].class, 25);
        boolean b3 = (o2.getClass() == int[][].class);
        boolean b4 = (o2.length == 25);

        var o3 = (int[][][]) Array.newInstance(int[][].class, 35);
        boolean b5 = (o3.getClass() == int[][][].class);
        boolean b6 = (o3.length == 35);
        return b1 && b2 && b3 && b4 && b5 && b6;
    }

    public static boolean arrayInit() {
        // The references inside the array are
        // automatically initialized to null.
        var os = new Object[5];
        for (var o: os)
            if (o != null)
                return false;

        // The primitives inside the array are
        // automatically initialized to zero.
        int[] is = new int[5];
        for (var i: is)
            if (i != 0)
                return false;

        return true;
    }

    public static boolean zeroLenArray() {
        int[] arr = new int[0];
        return (arr.getClass() == int[].class) && (arr.length == 0);
    }

    public static boolean multiDimArray() {
        int[][][] y = {
                {
                        {1},
                        {1, 2},
                        {1, 2, 3}
                }
        };

        boolean b1 = (y.length == 1);
        boolean b2 = (y[0].length == 3);
        boolean b3 = (y[0][0].length == 1);
        boolean b4 = (y[0][1].length == 2);
        boolean b5 = (y[0][2].length == 3);
        boolean b6 = (Arrays.deepToString(y).equals("[[[1], [1, 2], [1, 2, 3]]]"));
        return b1 && b2 && b3 && b4 && b5 && b6;
    }

    public static boolean newMultiDimArray() {
        int[][][] x = new int[2][3][5];
        boolean b1 = (x.length == 2);
        boolean b2 = (x[0].length == 3);
        boolean b3 = (x[1][2].length == 5);

        x[1][2][3] = 7;
        boolean b4 = (x[1][2][3] == 7);
        return b1 && b2 && b3 && b4;
    }

    public static boolean bigArray() {
        int[][][][][] arr = new int[3][4][5][6][7];
        for (int i = 0; i < arr.length; i++) {
            arr[i][0][0][0][0] = 3;
        }

        for (int i = 0; i < arr.length; i++) {
            if (arr[i][0][0][0][0] != 3)
                return false;
        }
        return true;
    }

    public static boolean longArray() {
        int len = 12345678;
        int[] arr = new int[len];
        for (int i = 0; i < len; i++) {
            arr[i] = i;
        }

        for (int i = 0; i < len; i++) {
            if (arr[i] != i)
                return false;
        }
        return true;
    }

    // 粗糙数组
    public static boolean raggedArray() {
        // 3-D array
        int[][][] a = new int[4][][];
        for (int i = 0; i < a.length; i++) {
            a[i] = new int[2][];
            for (int j = 0; j < a[i].length; j++) {
                a[i][j] = new int[3];
            }
        }
        return (Arrays.deepToString(a).equals("[[[0, 0, 0], [0, 0, 0]], [[0, 0, 0], [0, 0, 0]], [[0, 0, 0], [0, 0, 0]], [[0, 0, 0], [0, 0, 0]]]"));
    }

    public static boolean arrayCast() {
        try {
            int[][] arr = new int[2][3];
            Object[] o = (Object[]) arr; // OK
            int[][][] arr1 = new int[2][3][4];
            Object[] o1 = (Object[]) arr1; // OK
            Object[][] o2 = (Object[][]) arr1; // OK
        } catch (ClassCastException e) {
            return false;
        }
        return true;
    }

    public static boolean arrayCopy() {
        int len = 12345678;
        int[] arr = new int[len];
        for (int i = 0; i < len; i++) {
            arr[i] = i;
        }

        int[] copy = Arrays.copyOf(arr, arr.length);
        for (int i = 0; i < len; i++) {
            if (copy[i] != i)
                return false;
        }
        return true;
    }

    public static boolean arrayClass() {
        int[][] arr = new int[3][4];
        boolean b1 = (arr.getClass() == int[][].class);
        boolean b2 = (arr[0].getClass() == int[].class);
        boolean b3 = (arr[0].getClass() == arr[1].getClass());

        boolean b4 = (arr[0].getClass().getClassLoader() == null);
        boolean b5 = (int[].class.getClassLoader() == null);

        ArrayTest[][] arr1 = new ArrayTest[3][4];
        boolean b6 = (arr1.getClass() == ArrayTest[][].class);
        boolean b7 = (arr1[0].getClass() == ArrayTest[].class);
        boolean b8 = (arr1[0].getClass() == arr1[1].getClass());

        // 数组由其元素的类加载器加载
        ClassLoader loader = ArrayTest.class.getClassLoader();
        boolean b9 = (arr1[0].getClass().getClassLoader() == loader);
        boolean ba = (ArrayTest[].class.getClassLoader() == loader);
        boolean bb = (ArrayTest[][].class.getClassLoader() == loader);
        boolean bc = (ArrayTest[][][][][][][].class.getClassLoader() == loader);

        return b1 && b2 && b3 && b4 && b5 && b6 && b7 && b8 && b9 && ba && bb && bc;
    }

    public static boolean testArray() {
        int[] a1 = new int[10]; // newarray
        String[] a2 = new String[10]; // anewarray
        int[][] a3 = new int[10][10]; // multianewarray
        int[][][] a4 = new int[20][3][4]; // multianewarray
        a3[5][7] = 6;
        a4[4][2][1] = 4;
        int x = a1.length; // arraylength
        a1[0] = 100; // iastore
        int y = a1[0]; // iaload
        a2[0] = "abc"; // aastore
        String s = a2[0]; // aaload

        boolean b1 = (y == 100);
        boolean b2 = (a3[5][7] == 6);
        boolean b3 = (a4[4][2][1] == 4);
        boolean b4 = (x == 10);
        boolean b5 = (s.equals("abc"));
        return b1 && b2 && b3 && b4 && b5;
    }

    public static boolean bubbleSort() {
        int[] arr = { 22, 84, 77, 11, 95, 9, 78, 56, 36, 97, 65, 36, 10, 24, 92 };

        for (int i = 0; i < arr.length - 1; i++){
            for (int j = 0; j < arr.length - 1 - i; j++){
                if(arr[j] > arr[j + 1]){
                    int temp = arr[j];
                    arr[j] = arr[j + 1];
                    arr[j + 1] = temp;
                }
            }
        }

        return (Arrays.toString(arr).equals("[9, 10, 11, 22, 24, 36, 36, 56, 65, 77, 78, 84, 92, 95, 97]"));
    }

    public static void main(String[] args) {
        boolean b1 = ArrayTest.newArray();
        boolean b2 = ArrayTest.arrayInit();
        boolean b3 = ArrayTest.zeroLenArray();
        boolean b4 = ArrayTest.multiDimArray();
        boolean b5 = ArrayTest.newMultiDimArray();
        boolean b6 = ArrayTest.bigArray();
        boolean b7 = ArrayTest.longArray();
        boolean b8 = ArrayTest.raggedArray();
        boolean b9 = ArrayTest.arrayCast();
        boolean ba = ArrayTest.arrayCopy();
        boolean bb = ArrayTest.arrayClass();
        boolean bc = ArrayTest.testArray();
        boolean bd = ArrayTest.bubbleSort();

        if (b1 && b2 && b3 && b4 && b5 && b6 && b7 && b8 && b9 && ba && bb && bc && bd)
            System.out.println("passed");
        else
            System.out.println("failed");
    }
}
