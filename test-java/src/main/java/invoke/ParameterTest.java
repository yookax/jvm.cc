package invoke;

public class ParameterTest {
    private static void test(Object ...args) {
        System.out.println(args);
        for (int i = 0; i < args.length; i++) {
            System.out.println(args[i].getClass());
        }
    }

    public static void main(String[] args) {
        // test 里面怎么区分传递的是 int or Integer todo
        test("abc", 1, new Integer(2));
    }
}
