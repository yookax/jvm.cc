package lambda;

public class LambdaTest2 {
    public interface Bar {
        int run(int i);
    }

    public interface Bar2 {
        int run(int i, int j);
    }

    public interface Bar3 {
        Integer run(Integer i, Integer j);
    }

    public static void main(String[] args) {
        boolean flag = true;
        Bar b = i -> { i *= i; return i; };
        for (int i = 0; i < 10000; i++) {
            if (b.run(i) != i*i)
                flag = false;
        }

        Bar2 b2 = (i, j) -> { i += j; return i; };
        for (int i = 0; i < 10000; i++) {
            if (b2.run(i, i) != i+i)
                flag = false;
        }

        Bar3 b3 = (i, j) -> { i += j; return i; };
        for (int i = 0; i < 10000; i++) {
            if (b3.run(i, i+1) != i+i+1)
                flag = false;
        }

        System.out.println(flag);
    }
}
