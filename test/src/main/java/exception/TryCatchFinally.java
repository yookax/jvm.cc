package exception;

public class TryCatchFinally {

    static int func1() {
        int i = 1;
        try {
            return i;
        } finally {
            i = 2;
        }
    }

    static int func2() {
        int i = 1;
        try {
            return i;
        } finally {
            i = 2;
            return 2;
        }
    }

    public static void main(String[] args) {
        int i = func1();
        if (i != 1)
            System.out.println("failed");
        int j = func2();
        if (j != 2)
            System.out.println("failed");
    }
}
