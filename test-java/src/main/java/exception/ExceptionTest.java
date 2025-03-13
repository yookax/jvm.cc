package exception;

public class ExceptionTest {

    private static void foo() { bar(); }
    private static void bar() { bad(); }
    private static void bad() { throw new RuntimeException("BAD!"); }

    public static void testFinally() {
        int x = 1;
        try {
            bad();
            x = 100;
        } catch (Exception e) {
            x += 2;
        } finally {
            x *= 3;
        }

        if (x == (1 + 2) * 3) {
            System.out.println("Pass");
        }
    }

    public static void testStackTrace() throws InterruptedException {
        // sleep 防止和其他函数的异常信息打印混乱
        Thread.sleep(1000);
        try {
            foo();
        } catch (Exception e) {
            e.printStackTrace(System.err);
        }
        Thread.sleep(1000);
    }

    public static void testEmptyCatch() {
        try {
            bad();
        } catch (Exception e) {
            // empty
        }

        System.out.println("Pass");
    }

    public static void testUncaught() throws InterruptedException {
        // sleep 防止和其他函数的异常信息打印混乱
        Thread.sleep(1000);
        foo();
        Thread.sleep(1000);
    }

    public static void main(String[] args) throws InterruptedException {
        ExceptionTest.testFinally();
        ExceptionTest.testStackTrace();
        ExceptionTest.testEmptyCatch();
        ExceptionTest.testUncaught();
    }
}
