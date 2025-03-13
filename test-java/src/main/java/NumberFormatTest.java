import java.text.NumberFormat;
import java.util.Locale;

public class NumberFormatTest {
    public static void main(String[] args) {
        //支持压缩数字格式化
        NumberFormat cnf = NumberFormat.getCompactNumberInstance(Locale.US, NumberFormat.Style.SHORT);
        System.out.println(cnf.format(1_0000));
        System.out.println(cnf.format(1_9200));
        System.out.println(cnf.format(1_000_000));
        System.out.println(cnf.format(1L << 30));
        System.out.println(cnf.format(1L << 40));
        System.out.println(cnf.format(1L << 50));
    }
}
