# JVMCC
A tiny JVM written in C++.

## Development environment
* Win10
* cl
* jdk17
 
## Run
Ensure you already set `JAVA_HOME` environment variable.

One command-line option:
* -cp path: set class path.
```
C:\>jvmcc -cp D:\classes HelloWorld 
```
or, using CLASSPATH environment variable.
```
C:\>jvmcc HelloWorld
```
