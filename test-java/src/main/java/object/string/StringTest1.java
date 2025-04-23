package object.string;

public class StringTest1 {

    public static void main(String[] args) {
        String[] strings = {
                "Hello, World!",
                "你好，世界！",
                "こんにちは、世界！",
        };

        for (String s: strings)
            System.out.println(s);
    }
}
