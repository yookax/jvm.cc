
public class PrimaryTest {
    private long l;
    private double d;

    private long getLong() { return l; }
    private double getDouble() { return d; }

    public static void testLong() {
        var t = new PrimaryTest();
        long l = 8682522807148012L;
        t.l = l;
        System.out.println(t.getLong() == l ? "passed" : "failed");
    }

    public static void testDouble() {
        var t = new PrimaryTest();
        double d = 4349790087343.9483948938493;
        t.d = d;
        System.out.println(t.getDouble() == d ? "passed" : "failed");
    }

    public static void main(String[] args) {
        PrimaryTest.testLong();
        PrimaryTest.testDouble();
    }
}
