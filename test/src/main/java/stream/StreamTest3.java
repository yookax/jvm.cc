package stream;


import java.util.Arrays;
import java.util.Comparator;
import java.util.List;

class Trader {
    public final String name;
    public final String city;

    public Trader(String n, String c) {
        this.name = n;
        this.city = c;
    }

    public String toString() {
        return "Trader:" + name + " in " + city;
    }
}

class Transaction {
    public final Trader trader;
    public final int year;
    public final int value;

    public Transaction(Trader trader, int year, int value) {
        this.trader = trader;
        this.year = year;
        this.value = value;
    }

    public String toString() {
        return "{" + trader + ", " +
                "year: " + year + ", " +
                "value:" + value + "}";
    }
}

public class StreamTest3 {
    public static void main(String[] args) {
        Trader raoul = new Trader("Raoul", "Cambridge");
        Trader mario = new Trader("Mario","Milan");
        Trader alan = new Trader("Alan","Cambridge");
        Trader brian = new Trader("Brian","Cambridge");

        List<Transaction> transactions = Arrays.asList(
                new Transaction(brian, 2011, 300),
                new Transaction(raoul, 2012, 1000),
                new Transaction(raoul, 2011, 400),
                new Transaction(mario, 2012, 710),
                new Transaction(mario, 2012, 700),
                new Transaction(alan, 2012, 950)
        );

//        (1) 找出2011年发生的所有交易，并按交易额排序（从低到高）。
        System.out.println("--- 1 ---");
        transactions.stream()
                .filter(t -> t.year == 2011)
                .sorted(Comparator.comparingInt(t -> t.value))
                .forEach(System.out::println);

//        (2) 交易员都在哪些不同的城市工作过？
        System.out.println("--- 2 ---");
        transactions.stream()
                .map(t -> t.trader.city)
                .distinct()
                .forEach(System.out::println);

//        (3) 查找所有来自于剑桥的交易员，并按姓名排序。
        System.out.println("--- 3 ---");
        transactions.stream()
                .filter(t -> t.trader.city.equals("Cambridge"))
                .map(t -> t.trader.name)
                .distinct()
                .sorted(String::compareTo)
                .forEach(System.out::println);

//        (4) 返回所有交易员的姓名字符串，按字母顺序排序。
        System.out.println("--- 4 ---");
        System.out.println(transactions.stream()
                .map(t -> t.trader.name)
                .distinct()
                .sorted(String::compareTo)
                .reduce("", (s1, s2) -> s1 + s2));

//        (5) 有没有交易员是在米兰工作的？
        System.out.println("--- 5 ---");
        System.out.println(transactions.stream().anyMatch(t -> t.trader.city.equals("Milan")));

//        (6) 打印生活在剑桥的交易员的所有交易额。
        System.out.println("--- 6 ---");
        System.out.println(transactions.stream()
                .filter(t -> t.trader.city.equals("Cambridge"))
                .map(t -> t.value)
                .reduce(0, Integer::sum));

//        (7) 所有交易中，最高的交易额是多少？
        System.out.println("--- 7 ---");
        transactions.stream()
                .map(t -> t.value)
                .max(Integer::compareTo)
                .ifPresent(System.out::println);

//        (8) 找到交易额最小的交易。
        System.out.println("--- 8 ---");
        transactions.stream()
                .min(Comparator.comparingInt(t -> t.value))
                .ifPresent(System.out::println);
    }
}
