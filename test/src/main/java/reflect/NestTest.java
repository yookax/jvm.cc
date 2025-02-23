package reflect;

import java.util.Arrays;

// failed
public class NestTest {
    public static void main(String[] args) {
        System.out.println("---NestTest's NestHost");
        System.out.println(NestTest.class.getNestHost() == reflect.NestTest.class);
        System.out.println("---NestTest's NestMembers");
        System.out.println(Arrays.toString(NestTest.class.getNestMembers()));

        System.out.println("---D's NestHost");
        System.out.println(D.class.getNestHost() == reflect.NestTest.class);
        System.out.println("---D's NestMembers");
        System.out.println(Arrays.toString(D.class.getNestMembers()));

        System.out.println("---C's NestHost");
        System.out.println(C.class.getNestHost() == reflect.NestTest.class);
        System.out.println("---C's NestMembers");
        System.out.println(Arrays.toString(C.class.getNestMembers()));

        System.out.println("---D::E's NestHost");
        System.out.println(D.E.class.getNestHost() == reflect.NestTest.class);
        System.out.println("---D::E's NestMembers");
        System.out.println(Arrays.toString(D.E.class.getNestMembers()));
    }

    static class C { }

    static class D {
        static class E { }
    }
}
