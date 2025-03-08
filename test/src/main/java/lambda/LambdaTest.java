package lambda;

public class LambdaTest {
    public static void main(String[] args) {
        Runnable r = () -> {
            System.out.println("Simple lambda.");
        };
        r.run();
    }
    /*
      BootstrapMethods:
      0: #46 REF_invokeStatic java/lang/invoke/LambdaMetafactory.metafactory:(Ljava/lang/invoke/MethodHandles$Lookup;Ljava/lang/String;Ljava/lang/invoke/MethodType;Ljava/lang/invoke/MethodType;Ljava/lang/invoke/MethodHandle;Ljava/lang/invoke/MethodType;)Ljava/lang/invoke/CallSite;
          Method arguments:
            #53 ()V
            #54 REF_invokeStatic io/github/kayodesu/jdemos/dynamic/LambdaTest.lambda$main$0:()V
            #53 ()V

      Method main: // public static
            0 invokedynamic #7 <run, BootstrapMethods #0>
            5 astore_1
            6 aload_1
            7 invokeinterface #11 <java/lang/Runnable.run> count 1
            12 return

      Method lambda$main$0: // ()V, private static synthetic
            0 getstatic #15 <java/lang/System.out>
            3 ldc #21 <Simple lambda.>
            5 invokevirtual #23 <java/io/PrintStream.println>
            8 return

     */
}
