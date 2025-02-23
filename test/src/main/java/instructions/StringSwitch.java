package instructions;

public class StringSwitch {
    private static final String STR1 = "STR1";
    private static final String STR2 = "STR2";
    private static final String STR3 = "STR3";
    private static final String STR4 = "STR4";
    private static final String STR5 = "STR5";

    private static final String[] STRINGS = {
            "STR1", "STR2", "STR3", "STR4", "STR5", "abc"
    };

    public static void main(String[] args) {
        System.out.println("Expect output: 12345D");
        for (String s: STRINGS) {
            switch (new String(s)) { // 特意 new，以区分原 String
            case STR1 -> System.out.print("1");
            case STR5 -> System.out.print("5");
            case STR4 -> System.out.print("4");
            case STR2 -> System.out.print("2");
            case STR3 -> System.out.print("3");
            default -> System.out.print("D");
            }
        }
        System.out.println();
    }
}
