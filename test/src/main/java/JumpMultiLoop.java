/**
 * Status: Pass
 */
public class JumpMultiLoop {

    public static void main(String[] args) {
        a:
        while (true) {
            while (true) {
                while (true) {
                    break a;
                }
            }
        }

        System.out.println("passed");
    }
}
