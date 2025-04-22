/**
 *
 */
public class SwitchYield {

    public static void main(String[] args) {
        testSwitch1();
        testSwitch2();
        testSwitch3();
    }

    // 原始的switch
    private static void testSwitch1() {
        String x = "1";
        int i;
        switch (x) {
            case "1":
                i=1;
                break;
            case "2":
                i=2;
                break;
            default:
                i = x.length();
                break;
        }
        System.out.println(i);
    }

    //java 13：switch中引入yield，可以返回值给相应的变量
    private static void testSwitch2() {
        String x = "1";
        int i = switch(x){
            case "1" -> 5;
            case "2" -> 6;
            default -> {
                yield 7;
            }
        };
        System.out.println(i);
    }

    private static void testSwitch3() {
        String x = "3";
        int i = switch(x){
            case "1" : yield 5;
            case "2" : yield 6;
            default : yield 7;
        };
        System.out.println(i);
    }

    //yield 和 return
    //yield:结束switch结构
    //return:结束方法
}
