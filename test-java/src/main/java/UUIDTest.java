import java.util.UUID;

public class UUIDTest {
    public static void main(String[] args) throws ClassNotFoundException {
        UUID uuid = UUID.randomUUID();
        System.out.println("生成的 UUID: " + uuid);
    }
}
