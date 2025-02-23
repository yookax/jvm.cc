package instructions;

public class TableSwitch {
    public static void test(int i) {
        switch (i) {
        case 3 -> System.out.print("3");
        case 4 -> System.out.print("4");
        case 5 -> System.out.print("5");
        default -> System.out.print("D");
        }
    }
    
    public static void main(String[] args) {
        System.out.println("Expect output: D, D, 3, 4, 5, D");
        for (int i = 1; i < 7; i++) {
            test(i);
            if (i < 6)
                System.out.print(", ");
        }
        System.out.println();
    }
}
