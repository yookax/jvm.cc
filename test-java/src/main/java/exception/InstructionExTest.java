package exception;

/**
 * Expect Output:
 * java.lang.ClassCastException: java.lang.String cannot be cast to java.lang.Integer
 *     at exception.InstructionExTest.checkcast(InstructionExTest.java:44)
 * 	   at exception.InstructionExTest.main(InstructionExTest.java:32)
 * java.lang.NegativeArraySizeException
 *    	at exception.InstructionExTest.newarray(InstructionExTest.java:52)
 *    	at exception.InstructionExTest.main(InstructionExTest.java:33)
 * java.lang.NegativeArraySizeException
 * 	   at exception.InstructionExTest.anewarray(InstructionExTest.java:60)
 * 	   at exception.InstructionExTest.main(InstructionExTest.java:34)
 * java.lang.ArrayIndexOutOfBoundsException: 2
 *    	at exception.InstructionExTest.aload(InstructionExTest.java:69)
 * 	   at exception.InstructionExTest.main(InstructionExTest.java:35)
 * java.lang.ArrayIndexOutOfBoundsException: 1
 * 	   at exception.InstructionExTest.astore(InstructionExTest.java:18)
 * 	   at exception.InstructionExTest.main(InstructionExTest.java:36)
 * java.lang.ArithmeticException: / by zero
 * 	   at exception.InstructionExTest.idiv(InstructionExTest.java:88)
 * 	   at exception.InstructionExTest.main(InstructionExTest.java:37)
 * java.lang.ArithmeticException: / by zero
 * 	   at exception.InstructionExTest.irem(InstructionExTest.java:97)
 *     at exception.InstructionExTest.main(InstructionExTest.java:38)
 */
public class InstructionExTest {
    
    public static void main(String[] args) {
        checkcast();
        newarray();
        anewarray();
        aload();
        astore();
        idiv();
        irem();
    }

    public static void checkcast() {
        try {
            Object x = "String";
            Integer y = (Integer) x;
        } catch (ClassCastException e) {
            e.printStackTrace();
        }
    }

    public static void newarray() {
        try {
            int[] a = new int[-3];
        } catch (NegativeArraySizeException e) {
            e.printStackTrace();
        }
    }

    public static void anewarray() {
        try {
            Object[] a = new Object[-1];
        } catch (NegativeArraySizeException e) {
            e.printStackTrace();;
        }
    }

    public static void aload() {
        try {
            int[] a = {1};
            int x = a[2];
        } catch (ArrayIndexOutOfBoundsException e) {
            e.printStackTrace();
        }
    }

    public static void astore() {
        try {
            int[] a = {};
            a[1] = 2;
            int x = a[1];
        } catch (ArrayIndexOutOfBoundsException e) {
            e.printStackTrace();
        }
    }

    public static void idiv() {
        try {
            int x = 0;
            int y = 1 / x;
        } catch (ArithmeticException e) {
            e.printStackTrace();
        }
    }

    public static void irem() {
        try {
            int x = 0;
            int y = 1 % x;
        } catch (ArithmeticException e) {
            e.printStackTrace();
        }
    }
    
}
