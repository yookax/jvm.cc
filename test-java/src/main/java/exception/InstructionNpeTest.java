package exception;

/**
 * Status: Pass
 */
public class InstructionNpeTest {
    
    private int i;
    private Object o;

    public static void main(String[] args) throws Exception {
        arraylength();
        arrayLoad();
        arrayLoad1();
        arrayLoad2();
        athrow();
        getfield();
        getfield2();
        monitorenter();
        invokevirtual();
    }

    public static void arraylength() {
        try {
            int[] x = null;
            int y = x.length;
        } catch (NullPointerException e) {
            e.printStackTrace();
        }
    }

    public static void arrayLoad() {
        try {
            int[] x = null;
            int y = x[123456];
        } catch (NullPointerException e) {
            e.printStackTrace();
        }
    }

    public static void arrayLoad1() {
        try {
            int x = ((int[] )null)[0];
        } catch (NullPointerException e) {
            e.printStackTrace();
        }
    }

    public static void arrayLoad2() {
        try {
            Object[] x = null;
            Object y = x[123456];
        } catch (NullPointerException e) {
            e.printStackTrace();
        }
    }

    public static void athrow() throws Exception {
        try {
            Exception x = null;
            throw x;
        } catch (NullPointerException e) {
            e.printStackTrace();
        }
    }

    public static void getfield() {
        try {
            InstructionNpeTest x = null;
            int y = x.i;
        } catch (NullPointerException e) {
            e.printStackTrace();
        }
    }

    public static void getfield2() {
        try {
            InstructionNpeTest x = null;
            InstructionNpeTest z = null;
            Object y = x.o;
        } catch (NullPointerException e) {
            e.printStackTrace();
        }
    }

    public static void monitorenter() {
        try {
            Object x = null;
            synchronized(x) {
                System.out.println("BAD!");
            }
        } catch (NullPointerException e) {
            e.printStackTrace();
        }
    }

    public static void invokevirtual() {
        try {
            Object x = null;
            x.toString();
        } catch (NullPointerException e) {
            e.printStackTrace();
        }
    }
    
}
