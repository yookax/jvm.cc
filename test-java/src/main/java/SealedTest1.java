public class SealedTest1 {
    public static void main(String[] args) {
        Person1 employee = new Employee1(23, "Robert");
        System.out.println(employee.id());
        System.out.println(employee.name());
    }
}

sealed interface Person1 permits Employee1, Manager1 {
    int id();
    String name();
}

record Employee1(int id, String name) implements Person1 {}
record Manager1(int id, String name) implements Person1 {}