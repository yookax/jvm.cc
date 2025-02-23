package initialization;

public class ObjectInitTest {

    char x;
    int y = 2;
    static double a;
    static double b = 5.7;

    public static void main(String[] args) {
        ObjectInitTest o = new ObjectInitTest();
        
        char t0 = o.x;
        boolean b1 = (t0 == 0);
        
        int t2 = o.y;
        boolean b2 = (t2 == 2);

        double t = a;
        boolean b3 = (t == 0);
        
        double t1 = b;
        boolean b4 = (t1 == 5.7);
        System.out.println(b1 && b2 && b3 && b4);
    }

}
