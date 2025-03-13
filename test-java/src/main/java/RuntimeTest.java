
public class RuntimeTest {
    
    public static void main(String[] args) {
        Runtime rt = Runtime.getRuntime();
        System.out.println(rt);
        System.out.println("availableProcessors:" + rt.availableProcessors());
        System.out.println("freeMemory :" + rt.freeMemory());
        System.out.println("totalMemory:" + rt.totalMemory());
        System.out.println("maxMemory  :" + rt.maxMemory());
    }
    
}
