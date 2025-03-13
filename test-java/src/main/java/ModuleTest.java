public class ModuleTest {

    private static void printModuleInfo(Module m) {
        System.out.println(m);
        System.out.println(m.getClassLoader());
        System.out.println("descriptor: " + m.getDescriptor());
        System.out.println("layer: " + m.getLayer());
        System.out.println("packages: " + m.getPackages().size());
        System.out.println("------------------------");
    }

    public static void main(String[] args) {
        printModuleInfo(void.class.getModule());
        printModuleInfo(int.class.getModule());
        printModuleInfo(int[].class.getModule());
        printModuleInfo(ModuleTest.class.getModule());
        printModuleInfo(ModuleTest[].class.getModule());
    }
}
