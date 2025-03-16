package datetime;

import java.time.LocalTime;
import java.time.format.DateTimeFormatter;

public class DateTimeTest {
    public static void main(String[] args) {
        DateTimeFormatter dtf = DateTimeFormatter.ofPattern("B");
        System.out.println(dtf.format(LocalTime.of(8, 0)));
        System.out.println(dtf.format(LocalTime.of(13, 0)));
        System.out.println(dtf.format(LocalTime.of(20, 0)));
        System.out.println(dtf.format(LocalTime.of(23, 0)));
        System.out.println(dtf.format(LocalTime.of(0, 0)));
    }
}
