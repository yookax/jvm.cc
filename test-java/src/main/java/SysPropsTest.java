
public class SysPropsTest {
    
    public static void main(String[] args) {
        String[] keys = {
            "java.version",
            "java.vendor",
            "java.vendor.url",
            "java.home",
            "java.class.version",
            "java.class.path",
            "os.name",
            "os.arch",
            "os.version",
            "file.separator",
            "path.separator",
            "line.separator",
            "user.name",
            "user.home",
            "user.dir",
            "file.encoding",
            "sun.jnu.encoding",
            "sun.stdout.encoding",
            "sun.stderr.encoding",
        };

        for (String key : keys) {
            String val = System.getProperty(key);
            System.out.println(key + ": " + val);
        }

        System.out.println("-------------------------------------------------------");

        System.getProperties().forEach((key, val) -> System.out.println(key + ": " + val));
    }
    
}
