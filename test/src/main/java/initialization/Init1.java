package initialization;

/**
 * Dynamic Dispatch During Instance Creation
 */
public class Init1 {
    public static class Super {
        Super() { printThree(); }
        void printThree() { System.out.print("three"); }
    }
    public static class Test extends Super {
        int three = (int) Math.PI;  // That is 3
        void printThree() { System.out.print(three); }
    }

    public void test() {
        Test t = new Test();
        t.printThree();
    }
    
    public static void main(String[] args) {
        System.out.print("03 <---> ");
        new Init1().test();
    }
    
}
