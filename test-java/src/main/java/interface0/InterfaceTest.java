package interface0;// These codes are from: jdk: abstract class AbstractValidatingLambdaMetafactory

interface III<T> {
	Object foo(T x); 
}

interface JJ<R extends Number> extends III<R> { }

class CC { 
	String impl(int i) { 
		return "impl:" + i;
	}
	String impl(float f) {
		return "impl:" + f;
	}
}

public class InterfaceTest {
	public static void main(String[] args) {
		JJ<Integer> i = (new CC())::impl;
		System.out.println(i);
		System.out.printf(">>> %s\n\n", i.foo(44));

		JJ<Float> f = (new CC())::impl;
		System.out.println(f);
		System.out.printf(">>> %s\n", f.foo(3.5F));
	}
}
