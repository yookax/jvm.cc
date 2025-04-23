package object.string;

public class StringIndent {
    public static void main(String[] args) {
        // String中的indent()
        // 调整String实例的缩进
        String result = "Java\n  Python\nC++".indent(3);
        System.out.println(result);
    }
}
