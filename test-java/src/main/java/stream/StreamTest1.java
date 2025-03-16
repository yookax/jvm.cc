package stream;

import java.util.*;
import java.util.stream.Collectors;
import java.util.stream.Stream;

public class StreamTest1 {
    public static void main(String[] args) {
        StreamTest1.averagingInt();
        StreamTest1.averagingDouble();
        StreamTest1.averagingLong();
        StreamTest1.collectingAndThen();
        StreamTest1.counting();
        StreamTest1.joining();
        StreamTest1.summingInt();
        StreamTest1.summingLong();
        StreamTest1.summingDouble();
        StreamTest1.toList();
        StreamTest1.toSet();
        StreamTest1.toMap();
    }

    static void averagingInt() {
        List<Integer> list = Arrays.asList(1,2,3,4);
        Double result = list.stream().collect(Collectors.averagingInt(v->v*2));
        System.out.println(result);
    }

    static void averagingDouble() {
        List<Integer> list = Arrays.asList(1,2,3,4);
        Double result = list.stream().collect(Collectors.averagingDouble(d->d*2));
        System.out.println(result);
    }

    static void averagingLong() {
        List<Integer> list = Arrays.asList(1,2,3,4);
        Double result = list.stream().collect(Collectors.averagingLong(v->v*2));
        System.out.println(result);
    }

    static void collectingAndThen() {
        List<Integer> list = Arrays.asList(1,2,3,4);
        Double result = list.stream().collect(Collectors.collectingAndThen(Collectors.averagingLong(v->v*2),
                s-> s*s));
        System.out.println(result);
    }

    static void counting() {
        List<Integer> list = Arrays.asList(1,2,3,4);
        long result=  list.stream().collect(Collectors.counting());
        System.out.println(result);
    }

    static void joining() {
        List<String> list = Arrays.asList("A","B","C","D");
        String result=  list.stream().collect(Collectors.joining(",","(",")"));
        System.out.println(result);
    }

    static void summingInt() {
        List<Integer> list = Arrays.asList(30,10,20,35);
        int result = list.stream().collect(Collectors.summingInt(i->i));
        System.out.println(result);
    }

    static void summingLong() {
        List<Long> list = new ArrayList<>();
        list.add((long)340);
        list.add((long)240);
        list.add((long)360);
        long result = list.stream().collect(Collectors.summingLong(l->l));
        System.out.println(result);
    }

    static void summingDouble() {
        List<Double> list = Arrays.asList(340.5,234.56,672.76);
        Double result = list.stream().collect(Collectors.summingDouble(d->d));
        System.out.println(result);
    }

    static void toList() {
        List<String> list = Stream.of("AA","BB","CC").collect(Collectors.toList());
        list.forEach(s->System.out.println(s));
    }

    static void toSet() {
        Set<String> set = Stream.of("AA","AA","BB").collect(Collectors.toSet());
        set.forEach(s->System.out.println(s));
    }

    static void toMap() {
        Map<String,String> map = Stream.of("AA","BB","CC").collect(Collectors.toMap(k->k, v->v+v));
        map.forEach((k,v)->System.out.println("key:"+k +"  value:"+v));
    }
}