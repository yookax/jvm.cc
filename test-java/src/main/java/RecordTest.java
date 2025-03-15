
public record RecordTest(int i, long j) {

    public static void main(String[] args) {
        RecordTest t = new RecordTest(4, 98765432100L);
        System.out.println(
//                (RecordTest.k == 3) &&
                (t.i() == 4)
                && (t.i == 4)
                && (t.j() == 98765432100L)
                && (t.j == 98765432100L)
                && (t.getClass().getSuperclass() == Record.class)
                && t.toString().equals("RecordTest[i=4, j=98765432100]") ? "passed" : "failed");
    }
}
