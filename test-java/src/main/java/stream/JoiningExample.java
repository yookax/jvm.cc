package stream;

import java.util.Arrays;
import java.util.List;
import java.util.stream.Collectors;

public class JoiningExample {
    public static void main(String[] args) {
        List<String> list = Arrays.asList("p","a","s","s", "e", "d");
        String result=  list.stream().collect(Collectors.joining());
        System.out.println(result);
    }
}