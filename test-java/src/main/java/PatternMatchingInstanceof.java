/**
 * JEP 305：Pattern Matching for instanceof. instanceof 的模式匹配
 * https://openjdk.java.net/jeps/305
 */
public class PatternMatchingInstanceof {

    public static void main(String[] args) {
        // 原始 instanceof 方式
        Object obj = new String("hello,Java14");
        obj = null; // 在使用null 匹配instanceof 时，返回都是false.
        if(obj instanceof String){
            String str = (String) obj;
            System.out.println(str.contains("Java"));
        } else {
            System.out.println("非String类型");
        }

        // 新特性：省去了强制类型转换的过程
        // 此时的str的作用域仅限于if结构内。
        if(obj instanceof String str){
            System.out.println(str.contains("Java"));
        }else{
            System.out.println("非String类型");
        }

        // str = "abc"; 不能访问 str
    }
}

//举例3：
class Monitor {
    private String model;
    private double price;

//    public boolean equals(Object o){
//        if(o instanceof Monitor other){
//            if(model.equals(other.model) && price == other.price){
//                return true;
//            }
//        }
//        return false;
//    }

    public boolean equals(Object o){
        return o instanceof Monitor other && model.equals(other.model) && price == other.price;
    }
}