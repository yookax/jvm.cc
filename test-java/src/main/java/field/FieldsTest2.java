package field;

import java.lang.reflect.Field;

public class FieldsTest2 {
    public static final boolean z = true;

    public static void main(String[] args) throws NoSuchFieldException {
        Class<?> c = FieldsTest2.class;
        Field f = c.getField("z");
        System.out.println("Expect output: public static final boolean field.FieldsTest2.z");
        System.out.println(f);
    }
}
