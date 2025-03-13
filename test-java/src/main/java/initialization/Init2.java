package initialization;

/**
 * Superclasses Are Initialized Before Subclasses
 */
public class Init2 {
    public static class Super {
        static { System.out.print("Super "); }
    }
    public static class One {
        static { System.out.print("One "); }
    }
    public static class Two extends Super {
        static { System.out.print("Two "); }
    }

    public void test() {
        One o = new One();
        Two t = new Two();
        System.out.println((Object)o == (Object)t);
    }
    
    public static void main(String[] args) {
        System.out.print("One Super Two false <---> ");
        new Init2().test();
    }
    
}
