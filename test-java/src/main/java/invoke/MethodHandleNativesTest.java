package invoke;

import java.lang.reflect.Field;
import java.lang.reflect.Method;

public class MethodHandleNativesTest {
    
    public static void main(String[] args) {
        try {
            new MethodHandleNativesTest().getConstant();
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    public void getConstant() throws Exception {
        Class<?> mhnClass = Class.forName("java.lang.invoke.MethodHandleNatives");
        
        for (int i = 0; i < 100; i++) {
            Object x =  ReflectionHelper.call(mhnClass, "getConstant", i);
            if (i == 4) {
                System.out.println(1 == (Integer) x ? "Pass" : "Fail");
            } else {
                System.out.println(0 == (Integer) x ? "Pass" : "Fail");
            }
        }
    }

    static class ReflectionHelper {

        public static Object getStaticFieldValue(Class<?> c, String fieldName) throws ReflectiveOperationException {
            return getField(c, fieldName).get(null);
        }

        public static Object getFieldValue(Object obj, String fieldName) throws ReflectiveOperationException {
            return getField(obj.getClass(), fieldName).get(obj);
        }

        private static Field getField(Class<?> klass, String fieldName) {
            for (Class<?> c = klass; c != null; c = c.getSuperclass()) {
                try {
                    Field f = c.getDeclaredField(fieldName);
                    f.setAccessible(true);
                    return f;
                } catch (NoSuchFieldException e) {
                    // ignored
                }
            }
            return null;
        }

        // call static method
        public static Object call(Class<?> c, String methodName, Object... args) throws ReflectiveOperationException {
            for (Method m : c.getDeclaredMethods()) {
                if (m.getName().equals(methodName) && m.getParameterCount() == args.length) {
                    m.setAccessible(true);
                    return m.invoke(m, args);
                }
            }

            // todo
            return null;
        }

    }
    
}
