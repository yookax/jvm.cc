
// failed
public record RecordTest(int i) {

    public static void main(String[] args) {
        RecordTest t = new RecordTest(34);
        t.toString();
//        System.out.println(t);
//        System.out.println(
//                (RecordTest.k == 3)
//                && (t.i() == 4)
//                && (t.i == 4)
//                && (t.j() == 98765432100L)
//                && (t.j == 98765432100L)
//                && (t.getClass().getSuperclass() == Record.class)
//                && t.toString().equals("RecordTest[i=4, j=98765432100]"));
    }
}
