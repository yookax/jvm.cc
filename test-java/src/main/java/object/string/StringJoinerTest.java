package object.string;

import java.util.StringJoiner;

public class StringJoinerTest {
    public static void main(String[] args) {
        // 创建一个使用逗号作为分隔符的 StringJoiner
        StringJoiner joiner = new StringJoiner(",");
        joiner.add("apple");
        joiner.add("banana");
        joiner.add("cherry");
        System.out.println("使用逗号分隔: " + joiner.toString());

        // 创建一个使用逗号分隔，前后带有方括号的 StringJoiner
        StringJoiner joinerWithBrackets = new StringJoiner(",", "[", "]");
        joinerWithBrackets.add("red");
        joinerWithBrackets.add("green");
        joinerWithBrackets.add("blue");
        System.out.println("使用逗号分隔，带有方括号: " + joinerWithBrackets.toString());

        // 合并两个 StringJoiner
        StringJoiner mergedJoiner = joiner.merge(joinerWithBrackets);
        System.out.println("合并后的结果: " + mergedJoiner.toString());
    }
}
