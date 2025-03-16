import java.util.Base64;
import java.io.UnsupportedEncodingException;

public class Base64Test {

    public static void main(String args[]) {

        try {
            String base64encodedString = Base64.getEncoder().encodeToString(
                    "Java 8 Base64 编码解码".getBytes("utf-8"));
            System.out.println(base64encodedString);

            System.out.println();

            byte[] base64decodedBytes = Base64.getDecoder().decode(base64encodedString);

            System.out.println(new String(base64decodedBytes, "utf-8"));

        } catch(UnsupportedEncodingException e) {
            System.out.println(e.getMessage());
        }
    }
}