package object;

public class ObjectTest {
    public static int staticVar;
    public int instanceVar;

    public static boolean test() {
        int x = 32768; // ldc
        ObjectTest obj = new ObjectTest(); // new
        ObjectTest.staticVar = x; // putstatic
        x = ObjectTest.staticVar; // getstatic
        obj.instanceVar = x; // putfield
        x = obj.instanceVar; // getfield
        Object tmp = obj;
        if (tmp instanceof ObjectTest) { // instanceof
            obj = (ObjectTest) tmp; // checkcast
            return (obj.instanceVar == 32768) && (staticVar == 32768);
        } else {
            return false;
        }
    }

    public static boolean testEquals() {
        Class<ObjectTest> c1 = ObjectTest.class;
        Class<Integer> c2 = Integer.class;
        String s1 = "abc";
        String s2 = "123";
        return (!c1.equals(c2)) && (!s1.equals(s2));
    }

    private static class Super {
        int x;
        long y;
    }

    private static class Sub extends Super {
        float a;
        double b;
    }

    public static boolean testInheritance() {
        Sub sub = new Sub();
        sub.x = 1;
        sub.y = 2L;
        sub.a = 3.14f;
        sub.b = 2.71828;

        int x = sub.x;
        long y = sub.y;
        float a = sub.a;
        double b = sub.b;

        Super sup = sub;
        long j = sup.x + sup.y;

        return ((x == 1 && y == 2 && a == 3.14f && b == 2.71828 && j == 3L));
    }

    public static void main(String[] args) {
        System.out.println(test() && testEquals() && testInheritance() ? "passed" : "failed");
    }
}
