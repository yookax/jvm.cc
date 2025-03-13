package file;

import java.io.FileInputStream;
import java.io.FileNotFoundException;

public class FileNotFoundTest {
    
    public static void main(String[] args) {
        new FileNotFoundTest().fileNotFoundException();
    }

    public void fileNotFoundException() {
        try {
            new FileInputStream("abc/bcd/foo.txt");
        } catch (FileNotFoundException e) {
            e.printStackTrace();
        }
    }
    
}
