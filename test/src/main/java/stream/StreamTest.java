package stream;

import java.util.*;
import java.util.stream.*;

public class StreamTest {
	public static void main(String[] args) {
		// 将字符串换成大写并用逗号链接起来
		List<String> g7 = Arrays.asList("USA", "Japan", "France", "Germany", "Italy", "UK", "Canada");
		String g7Countries = g7.stream().map(String::toUpperCase).collect(Collectors.joining(", "));
		System.out.println(g7Countries);
	}
}