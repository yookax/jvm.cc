module;
#include <cassert>
#include "../../../vmdef.h"

module native;

import slot;
import classfile;
import object;
import encoding;
import runtime;
import class_loader;
import interpreter;
import exception;

using namespace std;
using namespace slot;
using namespace utf8;
using namespace java_lang_String;

/*
 * Cache the name to reduce the number of calls into the VM.
 * This field would be set by VM itself during initClassName call.
 *
 * private transient String name;
 *
 * private native String initClassName();
 */
void initClassName(Frame *f) {
    slot_t *args = f->lvars;
    auto _this = slot::get<jref>(args);
    auto c = _this->jvm_mirror;

    jstrRef class_name = Allocator::string(slash_2_dot_dup(c->name));
    class_name = intern(class_name);
    _this->set_field_value<jref>("name", "Ljava/lang/String;", class_name);
    f->pushr(class_name);
}

/*
 * Returns the {@code Class} representing the direct superclass of the
 * entity (class, interface, primitive type or void) represented by
 * this {@code Class}.  If this {@code Class} represents either the
 * {@code Object} class, an interface, a primitive type, or void, then
 * null is returned.  If this {@code Class} object represents an array class
 * then the {@code Class} object representing the {@code Object} class is
 * returned.
 *
 * @return the direct superclass of the class represented by this {@code Class} object
 */
// public native Class<? super T> getSuperclass();
void getSuperclass(Frame *f) {
    slot_t *args = f->lvars;
    auto _this = slot::get<jref>(args++)->jvm_mirror;

    jref sup = nullptr;
    if (_this->access_flags.isInterface()
            || _this->is_prim_class()
            || _this->check_class_name("void")
            || (_this->super_class == nullptr))
        sup = nullptr;
    else
        sup = _this->super_class->java_mirror;

    f->pushr(sup);
}

// private native Class<?>[] getInterfaces0();
void getInterfaces0(Frame *f) {
    slot_t *args = f->lvars;
    auto _this = slot::get<jref>(args);
    auto c = _this->jvm_mirror;

    auto size = (jint) c->interfaces.size();
    jarrRef interfaces = Allocator::class_array(size);
    for (int i = 0; i < size; i++) {
        assert(c->interfaces[i] != nullptr);
        interfaces->setRefElt(i, c->interfaces[i]->java_mirror);
    }

    f->pushr(interfaces);
}

/*
 * Determines if the class or interface represented by this
 * {@code Class} object is either the same as, or is a superclass or
 * superinterface of, the class or interface represented by the specified
 * {@code Class} parameter. It returns {@code true} if so;
 * otherwise it returns {@code false}. If this {@code Class}
 * object represents a primitive type, this method returns
 * {@code true} if the specified {@code Class} parameter is
 * exactly this {@code Class} object; otherwise it returns
 * {@code false}.
 *
 * <p> Specifically, this method tests whether the type represented by the
 * specified {@code Class} parameter can be converted to the type
 * represented by this {@code Class} object via an identity conversion
 * or via a widening reference conversion. See <cite>The Java Language
 * Specification</cite>, sections {@jls 5.1.1} and {@jls 5.1.4},
 * for details.
 *
 * @param     cls the {@code Class} object to be checked
 * @return    the {@code boolean} value indicating whether objects of the
 *            type {@code cls} can be assigned to objects of this class
 * @throws    NullPointerException if the specified Class parameter is
 *            null.
 * @since     1.1
 */
// public native boolean isAssignableFrom(Class<?> cls);
void isAssignableFrom(Frame *f) {
    slot_t *args = f->lvars;
    auto _this = slot::get<jref>(args++)->jvm_mirror;
    auto co = slot::get<jref>(args);
    if (co == nullptr) {
        throw java_lang_NullPointerException(); // todo msg
    }
    f->pushi(co->jvm_mirror->is_subclass_of(_this) ? 1 : 0);
}

// public native boolean isInterface();
void isInterface(Frame *f) {
    slot_t *args = f->lvars;
    auto _this = slot::get<jref>(args);
    auto c = _this->jvm_mirror;
    f->pushi(c->access_flags.isInterface() ? 1 : 0);
}

// public native boolean isArray();
void isArray(Frame *f) {
    slot_t *args = f->lvars;
    auto _this = slot::get<jref>(args);
    auto c = _this->jvm_mirror;
    f->pushi(c->is_array_class() ? 1 : 0);
}

// public native boolean isHidden();
void isHidden(Frame *f) {
    slot_t *args = f->lvars;
    auto _this = slot::get<jref>(args);
    auto c = _this->jvm_mirror;
    f->pushi(c->hidden ? 1 : 0);
}

// public native boolean isPrimitive();
void isPrimitive(Frame *f) {
    slot_t *args = f->lvars;
    auto _this = slot::get<jref>(args);
    auto c = _this->jvm_mirror;
    f->pushi(c->is_prim_class() ? 1 : 0);
}

// public native boolean isInstance(Object obj);
void isInstance(Frame *f) {
    slot_t *args = f->lvars;
    auto _this = slot::get<jref>(args++);
    auto obj = slot::get<jref>(args);
    f->pushi(obj->is_instance_of(_this->jvm_mirror) ? 1 : 0);
}

// public native int getModifiers();
void getModifiers(Frame *f) {
    slot_t *args = f->lvars;
    auto _this = slot::get<jref>(args);
    auto c = _this->jvm_mirror;
    f->pushi(c->access_flags.get()); // todo
}

//private native Field[] getDeclaredFields0(boolean publicOnly);
void getDeclaredFields0(Frame *frame) {
    slot_t *args = frame->lvars;
    auto _this = slot::get<jref>(args++);
    auto public_only = slot::get<jbool>(args);
    auto c = _this->jvm_mirror;

    Class *field_reflect_class = load_boot_class("java/lang/reflect/Field");

    Method *constructor;
    // Field(Class<?> declaringClass, String name, Class<?> type, int modifiers,
    //     boolean trustedFinal, int slot, String signature, byte[] annotations)
    constructor = field_reflect_class->get_constructor(
            "(Ljava/lang/Class;" "Ljava/lang/String;" "Ljava/lang/Class;"
            "IZI" "Ljava/lang/String;" "[B)V");

    u2 count = 0;
    auto objects = new Object*[c->fields.size()];

    // invoke constructor of class java/lang/reflect/Field
    for (size_t i = 0; i < c->fields.size(); i++) {
        auto f = c->fields[i];
        if (public_only && !f->access_flags.isPublic())
            continue;

        Object *o = Allocator::object(field_reflect_class);
        objects[count++] = o;
        auto sig = f->signature != nullptr ? Allocator::string(f->signature) : nullptr;

        execJava(constructor, {
            rslot(o),                                          // this
            rslot(c->java_mirror),                             // declaring class
            // name must be interned.
            // 参见 java/lang/reflect/Field 的说明
            rslot(intern(Allocator::string(f->name))),               // name
            rslot(f->get_type()),                               // type
            islot(f->access_flags.get()),                            // modifiers todo
            islot(f->access_flags.isFinal() ? jtrue : jfalse),              // trusted Final todo
            islot(f->access_flags.isStatic() ? i : f->id),                  // slot
            rslot(sig),                                        // signature
            rslot(get_annotation_as_byte_array(f->rt_visi_annos)), // annotations
        });
    }

    jarrRef field_array = Allocator::array(field_reflect_class->generate_array_class(), count);
    for (u2 i = 0; i < count; i++)
        field_array->setRefElt(i, objects[i]);

    delete[] objects;
    frame->pushr(field_array);
}

/*
 * 注意 getDeclaredMethods 和 getMethods 方法的不同。
 * getDeclaredMethods(),该方法是获取本类中的所有方法，包括私有的(private、protected、默认以及public)的方法。
 * getMethods(),该方法是获取本类以及父类或者父接口中所有的公共方法(public修饰符修饰的)
 *
 * getDeclaredMethods 强调的是本类中定义的方法，不包括继承而来的。
 * 不包括 class initialization method(<clinit>)和构造函数(<init>)
 */
//private native Method[] getDeclaredMethods0(boolean publicOnly);
void getDeclaredMethods0(Frame *f) {
    slot_t *args = f->lvars;
    auto _this = slot::get<jref>(args++);
    auto public_only = slot::get<jbool>(args);
    auto c = _this->jvm_mirror;

    Class *method_reflect_class = load_boot_class("java/lang/reflect/Method");

    /*
     * Method(Class<?> declaringClass, String name, Class<?>[] parameterTypes, Class<?> returnType,
     *      Class<?>[] checkedExceptions, int modifiers, int slot, String signature,
     *      byte[] annotations, byte[] parameterAnnotations, byte[] annotationDefault)
     */
    Method *constructor = method_reflect_class->get_constructor(
            "(Ljava/lang/Class;" "Ljava/lang/String;" "[" "Ljava/lang/Class;"
            "Ljava/lang/Class;" "[" "Ljava/lang/Class;" "II" "Ljava/lang/String;" "[B[B[B)V");

    u2 count = 0;
    auto objects = new Object*[c->methods.size()];

    for (size_t i = 0; i < c->methods.size(); i++) {
        Method *m = c->methods[i];
        if (public_only && !m->access_flags.isPublic())
            continue;
        if ((strcmp(m->name, "<clinit>") == 0) || (strcmp(m->name, "<init>") == 0))
            continue;

        Object *o = Allocator::object(method_reflect_class);
        objects[count++] = o;
        auto sig = m->signature != nullptr ? Allocator::string(m->signature) : nullptr;

        execJava(constructor, {
            rslot(o),                                               // this
            rslot(c->java_mirror),                                  // declaring class
            // name must be interned.
            // 参见 java/lang/reflect/Method 的说明
            rslot(intern(Allocator::string(m->name))),                    // name
            rslot(m->get_parameter_types()),                          // parameter types
            rslot(m->get_return_type()),                              // return type
            rslot(m->get_exception_types()),                          // checked exceptions
            islot(m->access_flags.get()),                                 // modifiers todo
            islot(i),                                               // slot
            rslot(sig),                                             // signature
            rslot(get_annotation_as_byte_array(m->rt_visi_annos)),      // annotations
            rslot(get_annotation_as_byte_array(m->rt_visi_para_annos)), // parameter annotations
            rslot(get_annotation_as_byte_array(m->annotation_default)), // annotation default
        });
    }

    jarrRef method_array = Allocator::array(method_reflect_class->generate_array_class(), count);
    for (size_t i = 0; i < count; i++) {
        method_array->setRefElt(i, objects[i]);
    }

    delete[] objects;
    f->pushr(method_array);
}

//private native Constructor<T>[] getDeclaredConstructors0(boolean publicOnly);
void getDeclaredConstructors0(Frame *f) {
    slot_t *args = f->lvars;
    auto _this = slot::get<jref>(args++);
    auto public_only = slot::get<jbool>(args);
    auto c = _this->jvm_mirror;

    Class *constructor_class = load_boot_class("java/lang/reflect/Constructor");

    vector<Method *> constructors = c->get_constructors(public_only);
    int count = constructors.size();

    jarrRef constructor_array = Allocator::array(constructor_class->generate_array_class(), count);

    /*
     * Constructor(Class<T> declaringClass, Class<?>[] parameterTypes,
     *      Class<?>[] checkedExceptions, int modifiers, int slot,
     *      String signature, byte[] annotations, byte[] parameterAnnotations)
     */
    Method *constructor = constructor_class->get_constructor(
            "(Ljava/lang/Class;" "[Ljava/lang/Class;"
            "[Ljava/lang/Class;" "IILjava/lang/String;" "[B[B)V");

    // invoke constructor of class java/lang/reflect/Constructor
    for (int i = 0; i < count; i++) {
        Method *cons = constructors[i];
        Object *o = Allocator::object(constructor_class);
        constructor_array->setRefElt(i, o);

        jstrRef sig = cons->signature != nullptr ? Allocator::string(cons->signature) : nullptr;

        execJava(constructor, {
            rslot(o),                                                  // this
            rslot(c->java_mirror),                                     // declaring class
            rslot(cons->get_parameter_types()),                          // parameter types
            rslot(cons->get_exception_types()),                          // checked exceptions
            islot(cons->access_flags.get()),                                 // modifiers todo
            islot(i),                                                  // slot
            rslot(sig),                                                // signature
            rslot(get_annotation_as_byte_array(cons->rt_visi_annos)),      // annotations
            rslot(get_annotation_as_byte_array(cons->rt_visi_para_annos)), // parameter annotations
        });
    }

    f->pushr(constructor_array);
}

// private native ProtectionDomain getProtectionDomain0();
void getProtectionDomain0(Frame *f) {
    slot_t *args = f->lvars;
    auto _this = slot::get<jref>(args);
    f->pushr(_this->protection_domain);
}

/*
 * Returns the Class object for the named primitive type. Type parameter T
 * avoids redundant casts for trusted code.
 */
//static native <T> Class<T> getPrimitiveClass(String name);
void getPrimitiveClass(Frame *f) {
    slot_t *args = f->lvars;
    // 这里的 class name 是诸如 "int, float" 之类的 primitive type
    auto name = slot::get<jref>(args);
    f->pushr(load_boot_class(java_lang_String::to_utf8(name))->java_mirror);
}

/*
 * getClasses 和 getDeclaredClasses 的区别：
 * getClasses 得到该类及其父类所有的 public 的内部类。
 * getDeclaredClasses 得到该类所有的内部类，除去父类的。
 */
// private native Class<?>[] getDeclaredClasses0();
void getDeclaredClasses0(Frame *f) {
    slot_t *args = f->lvars;
    auto _this = slot::get<jref>(args);
    auto c = _this->jvm_mirror;
    f->pushr(get_inner_classes_as_class_array(c, false));
}

// private native Class<?> getDeclaringClass0();
void getDeclaringClass0(Frame *f) {
    slot_t *args = f->lvars;
    auto _this = slot::get<jref>(args);
    auto c = _this->jvm_mirror;
    Class *d = get_declaring_class(c);
    f->pushr(d != nullptr ? d->java_mirror : nullptr);
}

/*
 * Returns the "simple binary name" of the underlying class, i.e.,
 * the binary name without the leading enclosing class name.
 * Returns null if the underlying class is a top level class.
 *
 * private native String getSimpleBinaryName0();
 */
void getSimpleBinaryName0(Frame *f) {
    slot_t *args = f->lvars;
    auto _this = slot::get<jref>(args);
    auto c = _this->jvm_mirror;

    // examples:
    // int -> int
    // java.lang.String -> String
    // pkg.Bar$Foo -> Foo

    const utf8_t *simple_binary_name = c->name;
    if (!c->is_prim_class()) {
        const char *p = strrchr(c->name, '$');
        if (p != nullptr) {
            simple_binary_name = ++p; // strip the leading enclosing class name
        } else if ((p = strrchr(c->name, '.')) != nullptr) {
            simple_binary_name = ++p; // strip the package name
        }
    }

    f->pushr(Allocator::string(simple_binary_name));
}

// Generic signature handling
//private native String getGenericSignature0();
void getGenericSignature0(Frame *f) {
    slot_t *args = f->lvars;
    auto _this = slot::get<jref>(args);
    auto c = _this->jvm_mirror;

    jref sig = nullptr;
    if (c->signature != nullptr)
        sig = Allocator::string(c->signature);
    f->pushr(sig);
}

// native byte[] getRawAnnotations();
void getRawAnnotations(Frame *f) {
    slot_t *args = f->lvars;
    auto _this = slot::get<jref>(args);
    auto c = _this->jvm_mirror;

    f->pushr(get_annotation_as_byte_array(c->rt_visi_annos));
}

// native byte[] getRawTypeAnnotations();
void getRawTypeAnnotations(Frame *f) {
    slot_t *args = f->lvars;
    auto _this = slot::get<jref>(args);
    auto c = _this->jvm_mirror;

    f->pushr(get_annotation_as_byte_array(c->rt_visi_type_annos));
}

// native ConstantPool getConstantPool();
void getConstantPool(Frame *f) {
    slot_t *args = f->lvars;
    auto _this = slot::get<jref>(args);
    auto c = _this->jvm_mirror;

    Class *cpc = load_boot_class("jdk/internal/reflect/ConstantPool");
    jref cp = Allocator::object(cpc);
    cp->set_field_value<jref>("constantPoolOop", "Ljava/lang/Object;", (jref)(c->cp));
    f->pushr(cp);
}

// Retrieves the desired assertion status of this class from the VM
// private static native boolean desiredAssertionStatus0(Class<?> clazz);
void desiredAssertionStatus0(Frame *f) {
    slot_t *args = f->lvars;
    // todo 本vm不讨论断言。desiredAssertionStatus0（）方法把false推入操作数栈顶
    f->pushi(jfalse);
}

/*
 * Generics reflection support.
 *
 * Returns information about the given class's EnclosingMethod
 * attribute, if present, or null if the class had no enclosing
 * method.
 *
 * If non-null, the returned array contains three elements. Element 0
 * is the java.lang.Class of which the enclosing method is a member,
 * and elements 1 and 2 are the java.lang.Strings for the enclosing
 * method's name and descriptor, respectively.
 */
// private native Object[] getEnclosingMethod0();
void getEnclosingMethod0(Frame *f) {
    slot_t *args = f->lvars;
    auto _this = slot::get<jref>(args);
    auto c = _this->jvm_mirror;

    if (c->enclosing.clazz == nullptr) {
        f->pushr(nullptr);
        return;
    }
    jarrRef a = Allocator::object_array({
        c->enclosing.clazz->java_mirror, c->enclosing.name, c->enclosing.descriptor });

    f->pushr(a);
}

// private native Class<?> getNestHost0();
void getNestHost0(Frame *f) {
    slot_t *args = f->lvars;
    auto _this = slot::get<jref>(args);
    auto c = _this->jvm_mirror;

    f->pushr(c->get_nest_host()->java_mirror);
}

// private native Class<?>[] getNestMembers0();
void getNestMembers0(Frame *f) {
    slot_t *args = f->lvars;
    auto _this = slot::get<jref>(args);
    auto c = _this->jvm_mirror;

    vector<void *> &members = c->get_nest_members();
    jarrRef o = Allocator::object_array(members.size());

    int i = 0;
    for (auto mem_cls: members) {
        assert(mem_cls != nullptr);
        o->setRefElt(i, ((Class *) mem_cls)->java_mirror);
        i++;
    }
    f->pushr(o);
}

/*
 * Returns an array containing the components of the Record attribute,
 * or null if the attribute is not present.
 *
 * Note that this method returns non-null array on a class with
 * the Record attribute even if this class is not a record.
 */
// private native RecordComponent[] getRecordComponents0();
void getRecordComponents0(Frame *f) {
    slot_t *args = f->lvars;
    auto _this = slot::get<jref>(args);
    auto c = _this->jvm_mirror;

    unimplemented
}

// private native boolean isRecord0();
void isRecord0(Frame *f) {
    slot_t *args = f->lvars;
    auto _this = slot::get<jref>(args);
    auto c = _this->jvm_mirror;

    unimplemented
}

// private native Class<?>[] getPermittedSubclasses0();
void getPermittedSubclasses0(Frame *f) {
    slot_t *args = f->lvars;
    auto _this = slot::get<jref>(args);
    auto c = _this->jvm_mirror;

    unimplemented
}

/*
 * Return the class's major and minor class file version packed into an int.
 * The high order 16 bits contain the class's minor version.  The low order
 * 16 bits contain the class's major version.
 *
 * If the class is an array type then the class file version of its element
 * type is returned.  If the class is a primitive type then the latest class
 * file major version is returned and zero is returned for the minor version.
 */
// private native int getClassFileVersion0();
void getClassFileVersion0(Frame *f) {
    slot_t *args = f->lvars;
    auto _this = slot::get<jref>(args);
    auto c = _this->jvm_mirror;

    unimplemented
}

/*
 * Return the access flags as they were in the class's bytecode, including
 * the original setting of ACC_SUPER.
 *
 * If the class is an array type then the access flags of the element type is
 * returned.  If the class is a primitive then ACC_ABSTRACT | ACC_FINAL | ACC_PUBLIC.
 */
// private native int getClassAccessFlagsRaw0();
void getClassAccessFlagsRaw0(Frame *f) {
    slot_t *args = f->lvars;
    auto _this = slot::get<jref>(args);
    auto c = _this->jvm_mirror;

    unimplemented
}

// Called after security check for system loader access checks have been made.
// private static native Class<?> forName0(String classname, boolean initialize,
//                        ClassLoader loader, Class<?> caller) throws ClassNotFoundException;
void forName0(Frame *f) {
    slot_t *args = f->lvars;
    auto classname = slot::get<jref>(args++);
    auto initialize = slot::get<jbool>(args++);
    auto loader = slot::get<jref>(args++);
    auto caller = slot::get<jref>(args);

    // todo
    Class *c = load_class(loader, java_lang_String::to_utf8(classname));
    if (c == nullptr) {
        throw java_lang_ClassNotFoundException(); // todo msg
        // todo ClassNotFoundException
        unimplemented
        // (*env)->ThrowNew(env, )
    }

    init_class(c);

    f->pushr(c->java_mirror);
}

// private static native void registerNatives();
void java_lang_Class_registerNatives(Frame *f) {
#undef R
#define R(method, method_descriptor) \
    registry("java/lang/Class", #method, method_descriptor, method)

    R(initClassName, "()Ljava/lang/String;");
    R(getSuperclass, "()Ljava/lang/Class;");
    R(getInterfaces0, "()[Ljava/lang/Class;");
    R(isAssignableFrom, "(Ljava/lang/Class;)Z");
    R(isInterface, "()Z");
    R(isArray, "()Z");
    R(isHidden, "()Z");
    R(isPrimitive, "()Z");
    R(isInstance, "(Ljava/lang/Object;)Z");
    R(getModifiers,  "()I");
    R(getDeclaredFields0, "(Z)[Ljava/lang/reflect/Field;");
    R(getDeclaredMethods0, "(Z)[Ljava/lang/reflect/Method;");
    R(getDeclaredConstructors0, "(Z)[Ljava/lang/reflect/Constructor;");
    R(getProtectionDomain0, "()Ljava/security/ProtectionDomain;");
    R(getPrimitiveClass, "(Ljava/lang/String;)Ljava/lang/Class;");
    R(getDeclaredClasses0, "()[Ljava/lang/Class;");
    R(getDeclaringClass0, "()Ljava/lang/Class;");
    R(getSimpleBinaryName0, "()Ljava/lang/String;");
    R(getGenericSignature0, "()Ljava/lang/String;");
    R(getRawAnnotations, "()[B");
    R(getRawTypeAnnotations, "()[B");
    R(getConstantPool, "()Ljdk/internal/reflect/ConstantPool;");
    R(desiredAssertionStatus0, "(Ljava/lang/Class;)Z");
    R(getEnclosingMethod0, "()[Ljava/lang/Object;");
    R(getNestHost0, "()Ljava/lang/Class;");
    R(getNestMembers0, "()[Ljava/lang/Class;");
    R(getRecordComponents0, "()[Ljava/lang/reflect/RecordComponent;");
    R(isRecord0, "()Z");
    R(getPermittedSubclasses0, "()[Ljava/lang/Class;");
    R(getClassFileVersion0, "()I");
    R(getClassAccessFlagsRaw0, "()I");
    R(forName0, "(Ljava/lang/String;ZLjava/lang/ClassLoader;Ljava/lang/Class;)Ljava/lang/Class;");
}