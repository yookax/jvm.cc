package interface0;

public class InterfaceField {
    private static interface A {
        int i = 1; // public static final
    }

    private static interface B {
        int i = 2;
    }

    private static interface C extends A { }

    private static interface D extends A, B {
        int i = 3;
    }

    public static void main(String[] args) {
        System.out.println(C.i == 1 && D.i == 3);
    }
}
