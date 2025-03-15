package field;

public class FieldsTest1 {
    
    static class Sup {
        private int a;

        public void setA(int a) {
            this.a = a;
        }

        public int getA() {
            return a;
        }
    }
    
    static class Sub extends Sup {
        int a;
    }

    public static void main(String[] args) {
        Sub sub = new Sub();
        sub.setA(1);
        sub.a = 2;
        boolean b1 = sub.getA() == 1;
        boolean b2 = sub.a == 2;
        System.out.println(b1 && b2 ? "passed" : "failed");
    }
    
}
