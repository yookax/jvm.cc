package object;

import java.util.ArrayList;
import java.util.List;

public class StringTest2 {
    public static void main(String[] args) {
        String sample = " abc ";
        System.out.println(sample.repeat(2)); // " abc  abc "
        System.out.println(sample.isBlank()); // false
        System.out.println("".isBlank()); // true
        System.out.println("   ".isBlank()); // true
        System.out.println(sample.strip()); // "abc"
        System.out.println(sample.stripLeading()); // "abc "
        System.out.println(sample.stripTrailing()); // " abc"
        sample = "This\nis\na\nmultiline\ntext.";

        List<String> lines = new ArrayList<>();

        sample.lines().forEach(line -> lines.add(line));
        lines.forEach(line -> System.out.println(line));
    }
}
