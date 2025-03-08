package stream;

import java.util.Arrays;
import java.util.List;
import java.util.stream.Collectors;
import java.util.stream.Stream;

class Dish {

    public enum Type { MEAT, FISH, OTHER }

    private final String name;
    private final boolean vegetarian;
    private final int calories;
    private final Type type;

    public Dish(String name, boolean vegetarian, int calories, Type type) {
        this.name = name;
        this.vegetarian = vegetarian;
        this.calories = calories;
        this.type = type;
    }

    public String getName() {
        return name;
    }

    public boolean isVegetarian() {
        return vegetarian;
    }

    public int getCalories() {
        return calories;
    }

    public Type getType() {
        return type;
    }

    @Override
    public String toString() {
        return name;
    }

}

public class StreamTest2 {

    static final List<Dish> menu = Arrays.asList(
            new Dish("pork", false, 800, Dish.Type.MEAT),
            new Dish("beef", false, 700, Dish.Type.MEAT),
            new Dish("chicken", false, 400, Dish.Type.MEAT),
            new Dish("french fries", true, 530, Dish.Type.OTHER),
            new Dish("rice", true, 350, Dish.Type.OTHER),
            new Dish("season fruit", true, 120, Dish.Type.OTHER),
            new Dish("pizza", true, 550, Dish.Type.OTHER),
            new Dish("prawns", false, 300, Dish.Type.FISH),
            new Dish("salmon", false, 450, Dish.Type.FISH) );

    void test0() {
        List<String> result =
                menu.stream()
                        .filter(dish -> dish.getCalories() > 300)
                        .map(Dish::getName)
                        .limit(3)
                        .collect(Collectors.toList());

        System.out.println(result);
    }

    void test1() {
        List<String> names =
                menu.stream()
                        .filter(d -> {
                            System.out.println("filtering " + d.getName());
                            return d.getCalories() > 300;
                        })
                        .map(d -> {
                            System.out.println("mapping " + d.getName());
                            return d.getName();
                        })
                        .limit(3)
                        .collect(Collectors.toList());
        System.out.println(names);
    }

    void test2() {
        List<String> title = Arrays.asList("Java8", "In", "Action");
        Stream<String> s = title.stream();
        s.forEach(System.out::println);
        s.forEach(System.out::println); // java.lang.IllegalStateException: stream has already been operated upon or closed
    }

    void test3() {
//        System.out.println(Arrays.toString("Java 8".split(""))); // [J, a, v, a,  , 8]

        List<String> words = Arrays.asList("Java 8", "Lambdas", "In", "Action");
        List<Character> result = words.stream()
                .map(word -> word.split(""))
                .flatMap(Arrays::stream)
                .distinct()
                .map(s -> s.charAt(0))
                .collect(Collectors.toList());
        System.out.println(result);
    }

    void test4() {
        List<Integer> ints1 = Arrays.asList(1, 2, 3);
        List<Integer> ints2 = Arrays.asList(3, 4);
        ints1.stream()
                .flatMap(i -> ints2.stream().filter(v -> (i + v)%3 == 0).map(v -> new Integer[]{i, v}))
                .forEach(arr -> System.out.println(Arrays.toString(arr)));
    }

    void test5() {
        var result = menu.stream().map(Dish::getName).limit(100).collect(Collectors.toList());
        System.out.println(result);

        result = menu.stream().map(Dish::getName).limit(3).collect(Collectors.toList());
        System.out.println(result);

        result = menu.stream().map(Dish::getName).skip(3).collect(Collectors.toList());
        System.out.println(result);

        result = menu.stream().map(Dish::getName).skip(3).limit(3).collect(Collectors.toList());
        System.out.println(result);

        result = menu.stream().map(Dish::getName).limit(3).skip(1).collect(Collectors.toList());
        System.out.println(result);

        result = menu.stream().map(Dish::getName).skip(1).limit(2).collect(Collectors.toList());
        System.out.println(result);
    }
}
