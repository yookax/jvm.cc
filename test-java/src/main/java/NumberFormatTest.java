import java.text.NumberFormat;
import java.util.Locale;

public class NumberFormatTest {
    public static void main(String[] args) {
        //支持压缩数字格式化
        class Foo {
            final long number;
            final String short_us;
            Foo(long number, String short_us) {
                this.number = number;
                this.short_us = short_us;
            }
        }
        
        Foo[] a = {
                new Foo(-10,"-10"),
                new Foo(0,"0"),
                new Foo(1_0000,"10K"),
                new Foo(1_9200,"19K"),
                new Foo(1_000_000,"1M"),
                new Foo(1L << 30,"1B"),
                new Foo(1L << 40,"1T"),
                new Foo(1L << 50,"1126T"),
        };

        NumberFormat cnf = NumberFormat.getCompactNumberInstance(Locale.US, NumberFormat.Style.SHORT);
        for (Foo foo : a) {
            String s = cnf.format(foo.number);
            if (!s.equals(foo.short_us)) {
                System.out.println("failed. " + foo.number + ", " + s);
                return;
            }
        }

        System.out.println("passed");
    }
}
