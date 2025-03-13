package file;

import java.io.File;

public class FileTest {
    
    public static void main(String[] args) {        
        File file = new File("test.txt");
        String p = file.getAbsolutePath();
        System.out.println("Should print absolute path below:");
        System.out.println(p);
    }
    
}
