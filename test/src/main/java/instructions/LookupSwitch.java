package instructions;

public class LookupSwitch {
    private static void test(int i) {
        switch (i) {
        case -100 -> System.out.print("-100");
        case 0 -> System.out.print("0");
        case 3 -> System.out.print("3");
        case -200 -> System.out.print("-200");
        case 5 -> System.out.print("5");
        default -> System.out.print("[" + i + "]");
        }
    }
    
    public static void main(String[] args) {
        System.out.println("Expect output: -100, -200, 0, [1], [2], 3, [4], 5, [6]");

        test(-100);
        System.out.print(", ");
        test(-200);
        System.out.print(", ");

        for (int i = 0; i < 7; i++) {
            test(i);
            if (i < 6)
                System.out.print(", ");
        }
    }
}
