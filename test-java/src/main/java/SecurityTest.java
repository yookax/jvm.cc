import java.security.PrivilegedAction;

class Foo {
    private static final Foo soleInstance = new Foo();

    public static Foo getFoo() {
        return soleInstance;
    }

    public static final class GetReflectionFactoryAction implements PrivilegedAction<Foo> {
        public Foo run() {
            return getFoo();
        }
    }
}

public class SecurityTest {
    private static Foo foo;

    private static Foo getReflectionFoo() {
        if (foo == null) {
            foo = java.security.AccessController.doPrivileged(new Foo.GetReflectionFactoryAction());
        }
        return foo;
    }

    public static void main(String[] args) {
        System.out.println(getReflectionFoo() != null ? "passed" : "failed");
    }
}
