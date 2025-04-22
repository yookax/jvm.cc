/**
 * JEP 368: TextBlock
 */
public class TextBlock {

    public static void main(String[] args) {
        test1();
        test2();
        test3();
        test4();
        test5();
        test6();
        test7();
        test8();
        test9();
        test10();
    }

    private static void test1() {
        // 原始的字符串
        String html = "<html>\n" +
                "  <body>\n" +
                "      <p>Hello, 世界！</p>\n" +
                "  </body>\n" +
                "</html>";
        System.out.println(html);

        System.out.println();

        // 使用 text block
        String html1 = """
<html>
  <body>
      <p>Hello, 世界！</p>
  </body>
</html>
""";
        System.out.println(html1);
    }

    private static void test2() {
        String sql = "select employee_id,last_name,salary,department_id\n" +
                "from employees\n" +
                "where department_id in (40,50,60)\n" +
                "order by department_id asc";

        System.out.println(sql);
        String sql1 = """
select employee_id,last_name,salary,department_id
from employees
where department_id in (40,50,60)
order by department_id asc
""";
        System.out.println(sql1);
    }

    //关于TextBlock的基本使用
    private static void test3() {
        //以开始分隔符的行终止符后的第一个字符开始
        //以结束分隔符的第一个双引号之前的最后一个字符结束
        String text1 = """
abc""";
        String text2 = "abc";
        System.out.println(text1 == text2);//text1和text2都指向了字符串常量池中唯一定义的abc字面量

        String text3 = """
abc
""";
        System.out.println(text1.length());//3
        System.out.println(text3.length());//4
    }

    //空字符串的表示
    private static void test4() {
        String text1 = "";
        System.out.println(text1.length());

        String text2 = """
""";
        System.out.println(text2.length());
    }

    //错误的写法
    private static void test5() {
//        String a = """""";   // 开始分隔符后没有行终止符
//        String b = """ """;  // 开始分隔符后没有行终止符
//        String c = """
//           ";        // 没有结束分隔符
//String d = """
//        abc \ def
//        """;      // 含有未转义的反斜线（请参阅下面的转义处理）
//
//        String e = "abc \ def";
    }

    //编译器在编译时会删除掉这些多余的空格
    private static void test6() {
        String text1 = """
    abc
    """;
        System.out.println(text1.length());//4
    }

    // 转义字符
    private static void test7() {
        String html = """
              <html>
                  <body>\n
                      <p>Hello, world</p>\n
                  </body>\n
              </html>\n
              """;
        System.out.println(html);
    }

    //在文本块内自由使用"是合法的
    private static void test8() {
        String story = """
    "When I use a word," Humpty Dumpty said,
    in rather a scornful tone, "it means just what I
    choose it to mean - neither more nor less."
    "The question is," said Alice, "whether you
    can make words mean so many different things."
    "The question is," said Humpty Dumpty,
    "which is to be master - that's all."
""";
        System.out.println(story);

        String code =
                """
    String text = \"""
        A text block inside a text block
    \""";
    """;
        System.out.println(code);
    }

    //文本块连接
    private static void test9() {
        String type = "String";
        String code = """
              public void print(""" + type + """
               o) {
                  System.out.println(Objects.toString(o));
              }
              """;
        System.out.println(code);

        //改进：可读性更好  ---方式1
        String code1 = """
              public void print($type o) {
                  System.out.println(Objects.toString(o));
              }
              """.replace("$type", type);
        System.out.println(code1);

        //方式2
        String code2 = String.format("""
              public void print(%s o) {
                  System.out.println(Objects.toString(o));
              }
              """, type);
        System.out.println(code2);

        //方式3
        String code3 = """
                public void print(%s object) {
                    System.out.println(Objects.toString(object));
                }
                """.formatted(type);
        System.out.println(code3);
    }

    private static void test10() {
        String sql1 = """
                SELECT id,NAME,email
                FROM customers
                WHERE id > 4
                ORDER BY email DESC
                """;
        System.out.println(sql1);

        // '\': 取消换行操作
        // '\s': 表示一个空格
        String sql2 = """
                SELECT id,NAME,email \
                FROM customers\s\
                WHERE id > 4 \
                ORDER BY email DESC
                """;
        System.out.println(sql2);
    }
}
