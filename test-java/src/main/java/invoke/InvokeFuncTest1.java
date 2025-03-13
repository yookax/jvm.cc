package invoke;

public class InvokeFuncTest1 {
    public static void main(String[] args) {
        System.out.println("Expect output: [012, 3], [123, 6], [234, 9], [345, 12]");
        for (int i = 0; i < 4; i++) {
            System.out.print(new InvokeFuncTest1().test(i, i + 1, i + 2));
            System.out.print("]");
            if (i < 3)
                System.out.print(", ");
        }
        System.out.println();
    }
    
    private int test(int i, int j, int k) {
        String s = "[" + i + "" + j + "" + k + ", ";
        System.out.print(s);
        return i + j + k;
    }
}
