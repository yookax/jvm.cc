package reflect;

public class ClassInitTest {
    
    static class A {
        public static int a = 100;
    }
    
    public static void main(String[] args) {
        ClassInitTest test = new ClassInitTest();
        test.getStatic();
    }

    public void getStatic() {
        try {
            Integer a = (Integer) A.class.getField("a").get(null);
            System.out.println(a == 100 ? "passed" : "failed");
        } catch (IllegalAccessException | NoSuchFieldException e) {
            e.printStackTrace();
        }
    }
    
}
