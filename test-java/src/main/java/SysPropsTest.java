
public class SysPropsTest {
    
    public static void main(String[] args) {
        System.getProperties().forEach((key, val) -> System.out.println(key + ": " + val));
    }
    
}
