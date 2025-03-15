package interface0;

public class InterfaceDefaultMethodTest {
    interface DefaultTest {
        default String test0() {
            return "DefaultTest";
        }
    }
    
    interface FirstTest extends DefaultTest { }

    interface SecondTest extends FirstTest {
        default String test0() {
            return "SecondTest";
        }
    }

    interface ThirdTest extends DefaultTest, SecondTest {
        default String test0() {
            return "ThirdTest";
        }
    }

    static class FirstTestClass implements ThirdTest, FirstTest { }

    static class TestInterfaceDefaultTest extends FirstTestClass implements FirstTest, SecondTest { }

    static class TestInterfaceFirstTestClass implements FirstTest, SecondTest, ThirdTest, DefaultTest { }

    private static boolean passed = true;

    public static void main(String[] args) {
        test1();
        test2();
        System.out.println(passed ? "passed" : "failed");
    }

    private static void checkResult(String s) {
        if (!s.equals("ThirdTest")) {
            passed = false;
        }
    }

    public static void test1() {
        TestInterfaceDefaultTest t = new TestInterfaceDefaultTest();
        checkResult(((DefaultTest) t).test0());
        checkResult(((FirstTestClass) t).test0());
        checkResult(((SecondTest) t).test0());
        checkResult(((ThirdTest) t).test0());
    }

    public static void test2() {
        TestInterfaceFirstTestClass t = new TestInterfaceFirstTestClass();
        checkResult(((DefaultTest) t).test0());
        checkResult(((FirstTest) t).test0());
        checkResult(((SecondTest) t).test0());
        checkResult(((ThirdTest) t).test0());
    }
}
