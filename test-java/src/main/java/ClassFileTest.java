import java.io.IOException;
import java.lang.classfile.ClassFile;
import java.lang.classfile.ClassModel;
import java.lang.classfile.constantpool.PoolEntry;
import java.nio.file.Files;
import java.nio.file.Path;

public class ClassFileTest {

    public static void main(String[] args) throws Exception {
        final ClassModel parse = ClassFile.of().parse(Path.of("out/production/untitled1/Test.class"));
        System.out.println(parse.majorVersion());
        System.out.println(parse.superclass().get().name());
        for (PoolEntry poolEntry : parse.constantPool()) {
            //System.out.println(STR."  \{poolEntry.toString()}");
        }
    }
}
