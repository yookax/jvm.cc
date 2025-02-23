package interface0;

public class InterfaceMethodTest {
    public interface If1 {
        static int x() {
            return 1;
        }
        default int y() {
            return 2;
        }
    }
    
    public static class Impl1 implements If1 {
        
    }
    
    public static class Impl2 implements If1 {        
        @Override
        public int y() {
            return 12;
        }
    }
    
    public static class Impl3 implements If1 {
        @Override
        public int y() {
            return 100 + If1.super.y(); //  invokespecial #2 <interface0/InterfaceMethodTest$If1.y>
        }
    }
    
    public static void main(String[] args) {
        InterfaceMethodTest t = new InterfaceMethodTest();
        System.out.println(t.defaultMethod() && t.staticMethod());
    }

    public boolean staticMethod() {
        return (1 == If1.x());
    }

    public boolean defaultMethod() {
        return (2 == new Impl1().y())
                && (12 == new Impl2().y())
                && (102 == new Impl3().y());
    }
}
