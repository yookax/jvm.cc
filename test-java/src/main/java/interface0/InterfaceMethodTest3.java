package interface0;

public class InterfaceMethodTest3 {
    interface Father {
        default String t() { return "Father"; }
    }

    interface Son extends Father {
        default String t() { return "Son"; }
    }

    private static class A implements Son { }

    private static class B extends A implements Father { }

    public static void main(String[] args) {
        System.out.println(new B().t().equals("Son") ? "passed" : "failed"); // Should be "Son" not "Father"
    }
}
