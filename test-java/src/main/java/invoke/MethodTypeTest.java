package invoke;

import java.lang.invoke.MethodType;

import static java.lang.invoke.MethodType.fromMethodDescriptorString;

public class MethodTypeTest {

    public static void main(String[] args) {

        String[] descriptors = {
                "()V",
                "(I)V",
                "(B)C",
                "(Ljava/lang/Integer;)V",
                "(Ljava/lang/Object;[[BLjava/lang/Integer;[Ljava/lang/Object;)V",
                "(II[Ljava/lang/String;)Ljava/lang/Integer;",
                "([Ljava/io/File;)Ljava/lang/Object;",
                "([[[Ljava/lang/Double;)[[Ljava/lang/Object;",
                "(ZBSIJFD)[[Ljava/lang/String;",
                "(ZZZZZZZZZZZZZZZZ)Z",
        };

        for (String desc: descriptors) {
            MethodType mt = fromMethodDescriptorString(desc, null);
            if (!desc.equals(mt.toMethodDescriptorString())) {
                System.out.println("Failed");
                return;
            }
        }
        System.out.println("Passed");
    }

}
