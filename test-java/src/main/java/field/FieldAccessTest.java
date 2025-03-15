package field;

public class FieldAccessTest {
    
    private static interface I {
        static int i = val(1);
    }
    private static interface J {
        static int j = val(2);
    }
    private static interface K extends I, J {
        static int k = val(3);
    }
    private static class A implements K {
        static int a = val(4);
    }
    private static class B extends A {
        static int b = val(5);
    }
    
    private static int val(int x) {
        return x;
    }

    public void test() {
        boolean b1 = (1 == B.i);
        boolean b2 = (2 == B.j);
        boolean b3 = (3 == B.k);
        boolean b4 = (4 == B.a);
        boolean b5 = (5 == B.b);
        System.out.println(b1 && b2 && b3 && b4 && b5 ? "passed" : "failed");
    }
    
    public static void main(String[] args) {
        new FieldAccessTest().test();
    }
    
}
