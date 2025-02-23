package reflect;

import java.util.Arrays;

public class DeclaringClassTest {
    
    static class A {
        static class B {
            class C {
                
            }
        }
    }

    public static void main(String[] args) {
        boolean b1 = (DeclaringClassTest.class.getName().equals("reflect.DeclaringClassTest"));
        boolean b2 = (DeclaringClassTest.class.getDeclaringClass() == null);
        boolean b3 = (Arrays.toString(DeclaringClassTest.class.getDeclaredClasses()).equals("[class reflect.DeclaringClassTest$A]"));

        boolean b4 = (A.class.getName().equals("reflect.DeclaringClassTest$A"));
        boolean b5 = (A.class.getDeclaringClass() == DeclaringClassTest.class);
        boolean b6 = (Arrays.toString(A.class.getDeclaredClasses()).equals("[class reflect.DeclaringClassTest$A$B]"));

        boolean b7 = (A.B.class.getName().equals("reflect.DeclaringClassTest$A$B"));
        boolean b8 = (A.B.class.getDeclaringClass() == A.class);
        boolean b9 = (Arrays.toString(A.B.class.getDeclaredClasses()).equals("[class reflect.DeclaringClassTest$A$B$C]"));

        boolean ba = (A.B.C.class.getName().equals("reflect.DeclaringClassTest$A$B$C"));
        boolean bb = (A.B.C.class.getDeclaringClass() == A.B.class);
        boolean bc = (A.B.C.class.getDeclaredClasses().length == 0);

        System.out.println(b1 && b2 && b3 && b4 && b5 && b6 && b7 && b8 && b9 && ba && bb && bc);
    }
    
}
