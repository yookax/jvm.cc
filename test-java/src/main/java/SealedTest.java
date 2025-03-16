
public class SealedTest {
    public static void main(String[] args) {
        Person manager = new Manager(23, "Robert");
        manager.name = "Robert";
        System.out.println(getId(manager));
    }

    public static int getId(Person person) {
        if (person instanceof Employee) {
            return ((Employee) person).getEmployeeId();
        } else if (person instanceof Manager) {
            return ((Manager) person).getManagerId();
        }
        return -1;
    }
}

abstract sealed class Person permits Employee, Manager {
    String name;

    String getName() {
        return name;
    }
}

final class Employee extends Person {
    String name;
    int id;

    Employee(int id, String name){
        this.id = id;
        this.name = name;
    }

    int getEmployeeId() {
        return id;
    }
}

non-sealed class Manager extends Person {
    int id;

    Manager(int id, String name){
        this.id = id;
        this.name = name;
    }

    int getManagerId() {
        return id;
    }
}