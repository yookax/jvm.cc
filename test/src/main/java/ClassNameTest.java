
public class ClassNameTest {

    private static class Foo { }

    public static void main(String[] args) {
        printName(void.class);
        printName(int.class);
        printName(String.class);
        printName(int[].class);
        printName(int[][].class);
        printName(String[].class);
        printName(String[].class);
        printName(ClassNameTest.class);
        printName(ClassNameTest[].class);
        printName(Foo.class);
        printName(Foo[].class);
    }

    private static void printName(Class<?> c) {
        System.out.println(c.getName() + " <-> " + c.getSimpleName());
    }
}
