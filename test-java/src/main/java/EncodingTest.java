import java.nio.charset.Charset;

public class EncodingTest {
    public static void main(String[] args) {
        // 获取默认字符编码
        Charset defaultCharset = Charset.defaultCharset();
        System.out.println("默认字符编码: " + defaultCharset);
    }
}
