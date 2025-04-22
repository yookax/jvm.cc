public class NestTest {
    public static void main(String[] args) {
        System.out.println("---NestTest's NestHost");
        System.out.println(NestTest.class.getNestHost());
        System.out.println("---NestTest's NestMembers");
        for(Class<?> cls: NestTest.class.getNestMembers())
            System.out.println(cls);

        System.out.println("---D's NestHost");
        System.out.println(D.class.getNestHost());
        System.out.println("---D's NestMembers");
        for(Class<?> cls: D.class.getNestMembers())
            System.out.println(cls);

        System.out.println("---C's NestHost");
        System.out.println(C.class.getNestHost());
        System.out.println("---C's NestMembers");
        for(Class<?> cls: C.class.getNestMembers())
            System.out.println(cls);

        System.out.println("---D::E's NestHost");
        System.out.println(D.E.class.getNestHost());
        System.out.println("---D::E's NestMembers");
        for(Class<?> cls: D.E.class.getNestMembers())
            System.out.println(cls);
    }

    static class C { }

    static class D {
        static class E { }
    }
}
