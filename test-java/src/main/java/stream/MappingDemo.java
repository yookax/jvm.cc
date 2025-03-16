package stream;

import java.util.ArrayList;
import java.util.List;
import java.util.Map;
import java.util.stream.Collectors;

public class MappingDemo {
    public static void main(String[] args) {
        List<Person> list = Person.getList();
        Map<Integer, String> nameByAge
                = list.stream().collect(Collectors.groupingBy(Person::getAge,
                Collectors.mapping(Person::getName, Collectors.joining(","))));
        nameByAge.forEach((k,v)->System.out.println("Age:"+k +"  Persons: "+v));
    }
}
class Person {
    private String name;
    private int age;
    public Person(String name, int age) {
        this.name = name;
        this.age = age;
    }
    public String getName() {
        return name;
    }
    public int getAge() {
        return age;
    }
    public static List<Person> getList() {
        List<Person> list = new ArrayList<>();
        list.add(new Person("Ram", 30));
        list.add(new Person("Shyam", 20));
        list.add(new Person("Shiv", 20));
        list.add(new Person("Mahesh", 30));
        return list;
    }
}
