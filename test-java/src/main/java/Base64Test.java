import java.nio.charset.StandardCharsets;
import java.util.Base64;

public class Base64Test {

    public static void main(String[] args) {

        String base64encodedString = Base64.getEncoder().encodeToString(
                "Java 8 Base64".getBytes(StandardCharsets.UTF_8));
        System.out.println(base64encodedString);
        System.out.println();

        byte[] base64decodedBytes = Base64.getDecoder().decode(base64encodedString);
        System.out.println(new String(base64decodedBytes, StandardCharsets.UTF_8));

    }
}