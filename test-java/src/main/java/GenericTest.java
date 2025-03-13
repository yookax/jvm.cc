import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

public class GenericTest {
    public static void erasedTypeEquivalence() {
        Class c1 = new ArrayList<String>().getClass();
        Class c2 = new ArrayList<String>().getClass();
        System.out.println(c1 == c2);
    }

    public static void lostInformation() {
        class Foo<T, K> {}
        var g = new Foo<Integer, String>();
        System.out.println(Arrays.toString(g.getClass().getTypeParameters()));
    }

    public static <T> List<T> makeList(T... args) {
        List<T> result = new ArrayList<>();
        for (T item : args) {
            result.add(item);
        }
        return result;
    }

    public static void main(String[] args) {
        erasedTypeEquivalence();
        lostInformation();

        List<String> list = makeList("A");
        System.out.println(list);

        list= makeList("A", "B", "C");
        System.out.println(list);

        list = makeList("ABCDEFG".split(""));
        System.out.println(list);
    }
}
