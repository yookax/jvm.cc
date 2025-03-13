import java.io.File;
import java.util.ArrayList;
import java.util.List;

public class RunAllTests {
    private final static String[] testCases = {
            "HelloWorld",
            "Ackermann",
            "ArrayTest",
            // "atomic.AtomicIntegerTest",
            "classloader.ClassLoader8Test",
            "classloader.ClassLoader9Test",
            "ClassNameTest",
            // "datetime.TimeZoneTest",
            // "EnumTest",
            "exception.CatchTest",
            // "exception.ClassLibExTest",
            "exception.FinallyReturnTest",
            "exception.InstructionExTest",
            "exception.InstructionNpeTest",
            // "exception.ExceptionTest",
            "FibonacciTest",
            // "field.ConstantStaticFieldsTest",
            // "field.FieldAccessTest",
            "field.FieldsTest",
            "field.FieldsTest1",
            "field.FieldsTest2",
            "file.FileDescriptorTest",
            // "file.FileNotFoundTest",
            "file.FileTest",
            "GenericTest",
            "initialization.Init1",
            "initialization.Init2",
            "initialization.Init3",
            "initialization.InitInterface",
            "initialization.InitStaticField",
            "initialization.ObjectInitTest",
            "instructions.AThrow",
            "instructions.CheckCast",
            "instructions.InvokeTest",
            "instructions.LookupSwitch",
            "instructions.MathTest",
            "instructions.StringSwitch",
            "instructions.TableSwitch",
            "instructions.WideInstruction",
            "interface0.GetInterfaceMethodTest",
            "interface0.InterfaceDefaultMethodTest",
            "interface0.InterfaceField",
            "interface0.InterfaceMethodTest",
            "interface0.InterfaceMethodTest2",
            "interface0.InterfaceMethodTest3",
            // "interface0.InterfaceTest",
            "invoke.InvokeFuncTest",
            "invoke.InvokeFuncTest1",
            // "invoke.MemberNameTest",
            "invoke.MethodHandleNativesTest",
            "invoke.MethodHandleTest",
            "invoke.MethodHandleTest1",
            // "invoke.MethodHandleTest2",
            "invoke.MethodTypeTest",
            "JumpMultiLoop",
            "lambda.LambdaTest",
            "lambda.LambdaTest1",
            "lambda.LambdaTest2",
            "lambda.LambdaTest3",
            // "method.ArgsPassTest",
            "ModuleTest",
            // "network.InetAddressTest",
            // "network.SocketConnectTest",
            // "network.SocketListenTest",
            // "network.UrlTest",
            "nio.ByteBufferTest",
            "object.ObjectTest",
            "object.WrapperTest",
            "PrimaryTest",
            // "PrintArgs",
            // "PrintTest",
            "QuickSortTest",
            // "RecordTest",
            "reflect.ArrayClassTest",
            // "reflect.ArrayGetTest",
            // "reflect.ArrayLengthTest",
            // "reflect.ArraySetTest",
            "reflect.ClassInitTest",
            "reflect.ClassTest",
            "reflect.DeclaringClassTest",
            "reflect.FieldTest",
            // "reflect.GenericTest",
            "reflect.MethodTest",
            "reflect.NestTest",
            "reflect.PrimitiveClassTest",
            // "thread.AliveTest",
            // "thread.DaemonTest",
            // "thread.DumpAllThreads",
            // "thread.InterruptFlagTest",
            // "thread.InterruptionTest",
            // "thread.MainThreadTest",
            // "thread.RunnableTest",
            // "thread.SleepTest",
            // "thread.SynchronizedTest",
            // "thread.ThreadSubClassTest",
            // "thread.WaitTest",
            // "ReflectTest",
            "RuntimeTest",
            "SecurityTest",
            "stream.StreamTest",
            "StringTest",
            "SysPropsTest",
             "JVMTest",
    };

    public static void main(String[] args) {
        printAlltests();

        for (String c: testCases) {
            try {
                Class<?> myClass = Class.forName(c);
                java.lang.reflect.Method method = myClass.getMethod("main", String[].class);
                System.out.println("<-------------------------- " + c + " -------------------------->");
                method.invoke(null, (Object) new String[0]);
            } catch (Exception e) { }
        }

        System.out.println("\n\nA total of " + testCases.length + " tests were run.");
    }

    public static void printAlltests() {
        File currentDirectory = new File(".");
        String currentJavaFileName = RunAllTests.class.getSimpleName() + ".java";
        List<String> filteredPaths = scanDirectory(currentDirectory, currentDirectory.getAbsolutePath(), currentJavaFileName);

        System.out.println(currentDirectory.getAbsolutePath());

        for (String path : filteredPaths) {
                    System.out.println("\"" + path + "\",");
        }
    }

    /**
     * 递归扫描目录及其子目录下的所有 Java 文件，并将过滤后的路径保存到列表中，排除当前文件
     * @param directory         要扫描的目录
     * @param basePath          基础路径，即当前目录的绝对路径
     * @param currentJavaFileName 当前 Java 文件的文件名
     * @return 过滤后的路径列表
     */
    public static List<String> scanDirectory(File directory, String basePath, String currentJavaFileName) {
        List<String> result = new ArrayList<>();
        if (directory.exists() && directory.isDirectory()) {
            // 获取目录下的所有文件和文件夹
            File[] files = directory.listFiles();
            if (files != null) {
                for (File file : files) {
                    if (file.isDirectory()) {
                        // 如果是目录，递归调用 scanDirectory 方法并将结果合并到当前列表
                        result.addAll(scanDirectory(file, basePath, currentJavaFileName));
                    } else {
                        // 检查文件是否为 Java 文件
                        if (file.getName().endsWith(".java") &&!file.getName().equals(currentJavaFileName)) {
                            String filePath = file.getAbsolutePath();
                            if (filePath.startsWith(basePath)) {
                                String relativePath = filePath.substring(basePath.length() + 1);
                                String filteredPath = filterPath(relativePath);
                                if (filteredPath != null) {
                                    // 将文件分隔符替换成.
                                    String finalPath = filteredPath.replaceAll("[/\\\\]", ".");
                                    result.add(finalPath);
                                }
                            }
                        }
                    }
                }
            }
        }
        return result;
    }

    /**
     * 过滤路径，保留 java\ 或 java/ 与 .java 之间的内容
     * @param path 原始路径
     * @return 过滤后的路径，如果不匹配则返回 null
     */
    public static String filterPath(String path) {
        int startIndexJava = Math.max(path.lastIndexOf("java\\"), path.lastIndexOf("java/"));
        if (startIndexJava != -1) {
            startIndexJava += 5; // "java\\" 或 "java/" 的长度
            int endIndex = path.lastIndexOf(".java");
            if (endIndex != -1) {
                return path.substring(startIndexJava, endIndex);
            }
        }
        return null;
    }
}
