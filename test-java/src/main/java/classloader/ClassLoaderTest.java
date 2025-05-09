package classloader;

import java.io.File;
import java.net.MalformedURLException;
import java.net.URL;
import java.net.URLClassLoader;

public class ClassLoaderTest {

    private static void printLoaderChain(ClassLoader loader) {
        System.out.println("--------------------------------");
        while (true) {
            System.out.println(loader);
            if (loader != null)
                loader = loader.getParent();
            else
                break;
        }
    }

    public static void main(String[] args) throws MalformedURLException {
        printLoaderChain(Object.class.getClassLoader());

        printLoaderChain(new URLClassLoader(new URL[]{ new File("d:/").toURI().toURL() }));

        printLoaderChain(ClassLoaderTest.class.getClassLoader());
    }
}
