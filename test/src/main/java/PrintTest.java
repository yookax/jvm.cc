public class PrintTest {
    public static void main(String[] args) {
        System.out.println("Expect output: false, 3, 3, a123456789, 9999999999, 122.445, 122.445");
        System.out.print(false);
        System.out.print(", ");
        System.out.print((byte) 3);
        System.out.print(", ");
        System.out.print((short) 3);
        System.out.print(", ");
        System.out.print('a');
        System.out.print(123456789);
        System.out.print(", ");
        System.out.print(9999999999L);
        System.out.print(", ");
        System.out.print(122.445F);
        System.out.print(", ");
        System.out.print(122.445);
        System.out.println();
        System.out.println();

        System.out.println("Expect output: 6.685785855285413E9, abc, 435, 959495.646460");
        System.out.print(6685785855.285412805887);

        String a = ", a";
        String b = "b";
        System.out.print(a + b + "c, ");

        System.out.printf("%d, %f\n", 435, 959495.64646);
    }
}
