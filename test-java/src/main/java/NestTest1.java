import java.util.Arrays;
import java.util.Set;
import java.util.stream.Collectors;

public class NestTest1 {
    public static void main(String[] args) {
        boolean isNestMate = NestTest1.class.isNestmateOf(NestTest1.Inner.class);
        boolean nestHost = NestTest1.Inner.class.getNestHost() == NestTest1.class;

        System.out.println(isNestMate);
        System.out.println(nestHost);

        Set<String> nestedMembers = Arrays.stream(NestTest1.Inner.class.getNestMembers())
                .map(Class::getName)
                .collect(Collectors.toSet());
        System.out.println(nestedMembers);
    }
    public class Inner{}
}