package initialization;

/**
 * Interface Initialization Does Not Initialize Superinterfaces
 */
public class InitInterface {
    public static interface I {
        int i = 1;
        int ii = out("ii", 2);
    }
    public static interface J extends I {
        int j = out("j", 3);
        int jj = out("jj", 4);
    }
    public static interface K extends J {
        int k = out("k", 5);
    }
    
    static int out(String s, int i) {
        System.out.print(s + "=" + i + " ");
        return i;
    }

    public void test() {
        System.out.print(J.i + " ");
        System.out.print(K.j + " ");
    }
    
    public static void main(String[] args) {
        System.out.print("1 j=3 jj=4 3 <---> ");
        new InitInterface().test();
        System.out.println();
    }
    
}
