import java.io.IOException;
import java.util.Collections;
import java.util.zip.ZipEntry;
import java.util.zip.ZipFile;

public class ZipFileTest {
    
    public static void main(String[] args) {
        var path = System.getProperty("java.home") + "/jmods/java.base.jmod";
        ZipFile zf = null;
        try {
            zf = new ZipFile(path);
        } catch (IOException e) {
            throw new RuntimeException(e);
        }

        System.out.println("java.base.jmod: " + zf.size());

        for (ZipEntry x : Collections.list(zf.entries())) {
           // System.out.println(x);
        }
    }
    
}
