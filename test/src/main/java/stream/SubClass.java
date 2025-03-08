package stream;

public class SubClass /*extends MyClass*/ implements MyFun, MyInterface{

	@Override
	public String getName() {
		return MyInterface.super.getName();
	}

}
