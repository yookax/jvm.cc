package classloader;

import java.io.File;
import java.net.MalformedURLException;
import java.net.URL;
import java.net.URLClassLoader;

/**
 *
 */
public class ClassLoader9Test {

    private static void printLoaderChain(ClassLoader loader) {
        System.out.println("------");
        while (true) {
            System.out.println(loader);
            if (loader != null)
                loader = loader.getParent();
            else
                break;
        }
    }

    public static void main(String[] args) throws MalformedURLException, ClassNotFoundException {
        ClassLoader loader1 = ClassLoader.getPlatformClassLoader();
        ClassLoader loader2 = ClassLoader.getSystemClassLoader();

        System.out.println(loader1);
        System.out.println(loader2);

        var c1 = loader1.loadClass("java.lang.Object");
        System.out.println(c1.getClassLoader());
        var c2 = loader2.loadClass("java.lang.Object");
        System.out.println(c1.getClassLoader());

//        c1 = loader1.loadClass("jdk.tools.jaotc.LoadedClass");
//        c1 = loader2.loadClass("jdk.tools.jaotc.LoadedClass");

        c1 = loader1.loadClass("java.awt.ActiveEvent");
        System.out.println(c1.getClassLoader());
        c1 = loader2.loadClass("java.awt.ActiveEvent");
        System.out.println(c1.getClassLoader());

        c1 = loader1.loadClass("sun.net.httpserver.AuthFilter");
        System.out.println(c1.getClassLoader());
        c1 = loader2.loadClass("sun.net.httpserver.AuthFilter");
        System.out.println(c1.getClassLoader());

        c1 = loader1.loadClass("java.sql.Array");
        System.out.println(c1.getClassLoader());
        c1 = loader2.loadClass("java.sql.Array");
        System.out.println(c1.getClassLoader());

        c1 = loader1.loadClass("sun.tools.attach.AttachProviderImpl");
        System.out.println(c1.getClassLoader());
        c1 = loader2.loadClass("sun.tools.attach.AttachProviderImpl");
        System.out.println(c1.getClassLoader());
/*
jdk.internal.loader.ClassLoaders$PlatformClassLoader@3f99bd52
jdk.internal.loader.ClassLoaders$AppClassLoader@6d06d69c
null
null
null
null
jdk.internal.loader.ClassLoaders$PlatformClassLoader@3f99bd52
jdk.internal.loader.ClassLoaders$PlatformClassLoader@3f99bd52
jdk.internal.loader.ClassLoaders$PlatformClassLoader@3f99bd52
jdk.internal.loader.ClassLoaders$PlatformClassLoader@3f99bd52
jdk.internal.loader.ClassLoaders$AppClassLoader@6d06d69c
jdk.internal.loader.ClassLoaders$AppClassLoader@6d06d69c
 */
//        printLoaderChain(Object.class.getClassLoader());
//        printLoaderChain(new URLClassLoader(new URL[]{ new File("d:/").toURI().toURL() }));
//        printLoaderChain(ClassLoader9Test.class.getClassLoader());
    }
}
