import java.util.HashSet;

/**
 *  JEP 359: Record
 */
public class Record1 {

    public static void main(String[] args) {
        test1();
        test2();
    }

    private static void test1() {
        //测试构造器
        Person p1 = new Person("罗密欧",new Person("zhuliye",null));
        //测试toString()
        System.out.println(p1);
        //测试equals():
        Person p2 = new Person("罗密欧",new Person("zhuliye",null));
        System.out.println(p1.equals(p2));

        //测试hashCode()和equals()
        HashSet<Person> set = new HashSet<>();
        set.add(p1);
        set.add(p2);

        for (Person person : set) {
            System.out.println(person);
        }

        //测试name()和partner():类似于getName()和getPartner()
        System.out.println(p1.name());
        System.out.println(p1.partner());

    }

    private static void test2() {
        Person p1 = new Person("zhuyingtai");

        System.out.println(p1.getNameInUpperCase());

        Person.nation = "CHN";
        System.out.println(Person.showNation());

    }

    static record Person(String name, Person partner) {

        // 还可以声明静态的属性、静态的方法、构造器、实例方法

        public static String nation;

        public static String showNation(){
            return nation;
        }

        public Person(String name){
            this(name,null);
        }

        public String getNameInUpperCase(){
            return name.toUpperCase();
        }

        // 不可以声明非静态的属性
        // private int id; //报错
    }

    // 不可以将 record 定义的类声明为 abstract 的
    // abstract record Order(){
    //
    // }

    // 不可以给 record 定义的类声明显式的父类（非Record类）
    // record Order() extends Thread {
    //
    // }

}


