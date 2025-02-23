package invoke;

public class InvokeFuncTest implements Runnable {
    public static void main(String[] args) {
        System.out.println("Expect output: staticMethod, instanceMethod, run, run.");
        new InvokeFuncTest().test();
    }

    public void test() {
        InvokeFuncTest.staticMethod(); // invokestatic
        InvokeFuncTest demo = new InvokeFuncTest(); // invokespecial
        demo.instanceMethod(); // invokespecial
        super.equals(null); // invokespecial
        this.run(); // invokevirtual
        System.out.print(", ");
        ((Runnable) demo).run(); // invokeinterface
        System.out.println(".");
    }

    public static void staticMethod() {
        System.out.print("staticMethod, ");
    }

    private void instanceMethod() {
        System.out.print("instanceMethod, ");
    }

    @Override
    public void run() {
        System.out.print("run");
    }
}
