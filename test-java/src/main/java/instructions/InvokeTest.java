package instructions;

import java.util.ArrayList;
import java.util.List;

//@SuppressWarnings("serial")
public class InvokeTest extends ArrayList<String> {
    public int x;

    public static void main(String[] args) throws Exception {
        InvokeTest test = new InvokeTest();
        test.invoke();
    }
    
    @Override
    public String get(int index) {
        return "InvokeTest";
    }
    
    public String getFromSuper(int index) {
        return super.get(index);
    }

    public void invoke() {
        InvokeTest test = new InvokeTest();
        test.add("ArrayList");
        
        // invokevirtual
        ArrayList<String> arrayList = test;
    //    assertEquals("InvokeTest", arrayList.get(0));
        
        // invokespecial
     //   assertEquals("ArrayList", test.getFromSuper(0));
        
        // invokeinterface
        List<String> list = test;
    //    assertEquals("InvokeTest", list.get(0));
    }
//    
//    
//    public void invokeVirtual() throws Exception {
//        Constructor<FrameworkField> c = FrameworkField.class.getDeclaredConstructor(Field.class);
//        c.setAccessible(true);
//        Field f = InvokeTest.class.getField("x");
//        FrameworkField ff = c.newInstance(f);
//        
//        Method m = InvokeTest.class.getMethod("invokeVirtual");
//        FrameworkMethod fm = new FrameworkMethod(m);
//        
//        FrameworkMember<?> a = ff;
//        FrameworkMember<?> b = fm;
//        
//        b.getAnnotations();
//        a.getAnnotations();
//    }
//    
//    
//    public void invokeVirtual2() {
//        abstract class A implements Runnable {}
//        abstract class B implements Runnable {
//             @Override public abstract void run();
//        }
//        class C extends A {
//            @Override public void run(){}
//        }
//        
//        A a = new C();
//        a.run();
//    }
    
}
