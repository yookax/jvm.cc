package reflect;

import java.lang.reflect.Method;
import java.util.ArrayList;
import java.util.Arrays;

public class ClassTest implements Runnable {
    private int a;
    public double b;
    static boolean z;

    private static final ArrayList<Boolean> list = new ArrayList<>();

    public static void main(String[] args) throws Exception {
        _package();
        _class();
        method();

        for (boolean b: list) {
            if (!b) {
                System.out.println("Failed");
                return;
            }
        }
        System.out.println("Passed");
    }

    public static void _package() {
        list.add("reflect".equals(ClassTest.class.getPackage().getName()));
    }

    public static void _class() {
        Class<?> c = ClassTest.class;
        list.add("reflect.ClassTest".equals(c.getName()));
        list.add(Object.class == c.getSuperclass());
        list.add(Arrays.deepEquals(new Class<?>[]{Runnable.class}, c.getInterfaces()));
        list.add(1 == c.getFields().length);
        list.add(4 == c.getDeclaredFields().length);
        list.add(14 == c.getMethods().length);
        list.add(5 == c.getDeclaredMethods().length);
    }

    public static void method() throws Exception {
        Method main = ClassTest.class.getMethod("main", String[].class);
        list.add(Arrays.deepEquals(new Class<?>[]{Exception.class}, main.getExceptionTypes()));
        list.add(Arrays.deepEquals(new Class<?>[]{String[].class}, main.getParameterTypes()));
        list.add(0 == main.getDeclaredAnnotations().length);

        Method run = ClassTest.class.getMethod("run");
        list.add(0 == run.getDeclaredAnnotations().length);
    }

    @Override
    public void run() { }
}
