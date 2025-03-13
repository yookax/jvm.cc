public class StringTest {
    public static void test() {
        String s1 = new String("abc1");
        System.out.println(s1);
        String s2 = "abc1";

        int x = 1;
        String s3 = "abc" + x;
        System.out.println(s3);
        if (s1.equals(s3)) {
            System.out.println("true");
        } else {
            System.out.println("false");
        }

        s3 = s3.intern();
        String s4 = "abc1";
        if (s3 == s4) {
            System.out.println("true");
        } else {
            System.out.println("false");
        }
    }

    public static void testIntern() {
        String s = "ABC";

        String a = new String("ABC");
        String b = new String("ABC");
        System.out.println((a != b) ? "Pass":"Fail");

        String interned = a.intern();
        System.out.println((interned == s)  ? "Pass":"Fail");
        System.out.println((interned == b.intern())  ? "Pass":"Fail");
    }

    public static void main(String[] args) {
        int x = 1;
        String s = "abc" + x;
        System.out.println(s);
        StringTest.test();
        StringTest.testIntern();
    }
}
