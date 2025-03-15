package interface0;

public class InterfaceMethodTest2 {

    interface Default {
        default String t() {
            return "Default";
        }
    }

    interface Itf1 extends Default {
        default String t() {
            return "Interface1";
        }
    }

    interface Itf2 extends Itf1 {
        default String t() {
            return "Interface2";
        }
    }

    private static class A implements Itf2 { }

    private static class B extends A implements Itf1 { }

    public static void main(String[] args) {
        B b = new B();
        A a = (A) b;
        Itf1 itf1 = (Itf1) b;
        Itf2 itf2 = (Itf2) b;
        System.out.println(a.t().equals("Interface2")
                && itf1.t().equals("Interface2")
                && itf2.t().equals("Interface2") ? "passed" : "failed");
    }
}
