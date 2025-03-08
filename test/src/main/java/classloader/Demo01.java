package classloader;

public class Demo01 {
	static{
		System.out.println("静态初始化Demo01");
	}

	public static void main(String[] args) throws Exception {
		System.out.println("Demo01的main方法！");
		System.out.println(System.getProperty("java.class.path"));
		
		//主动引用
//		new A();
//		System.out.println(A.width);
//		Class.forName("com.bjsxt.test.A");
		
		
		//被动引用
//		System.out.println(A.MAX);
//		A[] as = new A[10];
		System.out.println(B.width);
		
	}
}

class B  extends A {
	static {
		System.out.println("静态初始化B");
	}
}

class A extends A_Father {
	public static int width=100;   //静态变量，静态域    field
	public static final  int MAX=100; 
	
	static {
		System.out.println("静态初始化类A");
		width=300;
	}
	public A(){
		System.out.println("创建A类的对象");
	}
}

class A_Father extends Object {
	static {
		System.out.println("静态初始化A_Father");
	}
}
