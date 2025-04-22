/**
 * JEP 361: switch expression
 */
public class SwitchExpression {

    public static void main(String[] args) {
        testSwitch1();
        testSwitch2();
    }

    private enum Fruit {
        PEAR, APPLE, GRAPE, MANGO, ORANGE, PAPAYA;
    }

    private static void testSwitch1() {
        Fruit fruit = Fruit.APPLE;
        switch(fruit) {
        case PEAR -> System.out.println("4");
        case APPLE,GRAPE,MANGO -> System.out.println("5");
        case ORANGE,PAPAYA -> System.out.println("6");
        default -> throw new IllegalStateException("No Such Fruit:" + fruit);
        }
    }

    private static void testSwitch2() {
        int numberOfLetters;
        Fruit fruit = Fruit.APPLE;
        // switch可返回一个值
        numberOfLetters = switch(fruit) {
            case PEAR -> 4;
            case APPLE,GRAPE,MANGO -> 5;
            case ORANGE,PAPAYA -> 6;
            default -> throw new IllegalStateException("No Such Fruit:" + fruit);
        };
        System.out.println(numberOfLetters);

    }
}
