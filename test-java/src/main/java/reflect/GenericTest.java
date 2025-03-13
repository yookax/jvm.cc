package reflect;

import java.lang.reflect.Field;
import java.util.ArrayList;
import java.util.List;

public class GenericTest {
    private static class GenericClass<String> { }

    private static final String str = "abc";
    private static final List<String> list = new ArrayList<>();

    public static boolean typeParameter() {
        var x = GenericClass.class.getTypeParameters();
        boolean b1 = (x.length == 1);
        boolean b2 = (x[0].toString().equals("String"));
        return b1 && b2;
    }

    public static boolean fieldGenericType() throws NoSuchFieldException {
        boolean b1 = (String.class == GenericTest.class.getDeclaredField("str").getGenericType());
        Field listField = GenericTest.class.getDeclaredField("list");
        boolean b2 = (listField.getName().equals("list"));
        boolean b3 = (List.class == listField.getType());
        boolean b4 = (listField.getModifiers() == 26); // 26 /* private static final */
        boolean b5 = (listField.getGenericType().toString().equals("java.util.List<java.lang.String>"));
        return b1 && b2 && b3 && b4 && b5;
    }

    public static void main(String[] args) throws Exception {
        System.out.println(typeParameter() && fieldGenericType());
    }
}
