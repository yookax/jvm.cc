module;
#include "vmdef.h"

export module exception;

export struct UncaughtJavaException: public std::exception {
    Object *java_excep;
    explicit UncaughtJavaException(Object *java_excep0): java_excep(java_excep0) { }
};

export struct JavaException: public std::exception {
    const char *excep_class_name = nullptr;
    Object *excep = nullptr;
    std::string msg;

    explicit JavaException(const char *excep_class_name0): excep_class_name(excep_class_name0) {
        assert(excep_class_name != nullptr);
    }

    explicit JavaException(const char *excep_class_name0, std::string msg0)
            : excep_class_name(excep_class_name0), msg(msg0) {
        assert(excep_class_name != nullptr);
    }

    Object *get_excep();
};

#define DEF_EXCEP_CLASS(ClassName, ClassNameStr) \
export struct ClassName: public JavaException { \
    ClassName(): JavaException(ClassNameStr) { } \
    explicit ClassName(std::string msg0): JavaException(ClassNameStr, msg0) { } \
}

DEF_EXCEP_CLASS(java_lang_ArrayStoreException, "java/lang/ArrayStoreException");
DEF_EXCEP_CLASS(java_lang_UnknownError, "java/lang/UnknownError");
DEF_EXCEP_CLASS(java_lang_ArrayIndexOutOfBoundsException, "java/lang/ArrayIndexOutOfBoundsException");
DEF_EXCEP_CLASS(java_lang_ArithmeticException, "java/lang/ArithmeticException");
DEF_EXCEP_CLASS(java_lang_ClassNotFoundException, "java/lang/ClassNotFoundException");
DEF_EXCEP_CLASS(java_lang_InternalError, "java/lang/InternalError");
DEF_EXCEP_CLASS(java_lang_IncompatibleClassChangeError, "java/lang/IncompatibleClassChangeError");
DEF_EXCEP_CLASS(java_lang_IllegalAccessError, "java/lang/IllegalAccessError");
DEF_EXCEP_CLASS(java_lang_AbstractMethodError, "java/lang/AbstractMethodError");
DEF_EXCEP_CLASS(java_lang_InstantiationException, "java/lang/InstantiationException");
DEF_EXCEP_CLASS(java_lang_NegativeArraySizeException, "java/lang/NegativeArraySizeException");
DEF_EXCEP_CLASS(java_lang_NullPointerException, "java/lang/NullPointerException");
DEF_EXCEP_CLASS(java_lang_ClassCastException, "java/lang/ClassCastException");
DEF_EXCEP_CLASS(java_lang_ClassFormatError, "java/lang/ClassFormatError");
DEF_EXCEP_CLASS(java_lang_LinkageError, "java/lang/LinkageError");
DEF_EXCEP_CLASS(java_lang_NoSuchFieldError, "java/lang/NoSuchFieldError");
DEF_EXCEP_CLASS(java_lang_NoSuchMethodError, "java/lang/NoSuchMethodError");
DEF_EXCEP_CLASS(java_lang_IllegalArgumentException, "java/lang/IllegalArgumentException");
DEF_EXCEP_CLASS(java_lang_CloneNotSupportedException, "java/lang/CloneNotSupportedException");
DEF_EXCEP_CLASS(java_lang_VirtualMachineError, "java/lang/VirtualMachineError");
DEF_EXCEP_CLASS(java_io_IOException, "java/io/IOException");
DEF_EXCEP_CLASS(java_io_FileNotFoundException, "java/io/FileNotFoundException");
DEF_EXCEP_CLASS(sun_nio_fs_WindowsException, "sun/nio/fs/WindowsException");

//export struct sun_nio_fs_WindowsException: public JavaException {
//    sun_nio_fs_WindowsException(): JavaException("sun/nio/fs/WindowsException") { }
//
//    explicit sun_nio_fs_WindowsException(std::string msg0)
//            : JavaException("sun/nio/fs/WindowsException", msg0) { }
//}

#undef DEF_EXCEP_CLASS

export void print_stack_trace(Object *e);
