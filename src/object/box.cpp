module;
#include "../classfile/class_loader.h"

module object;

import std.core;

using namespace std;
using namespace slot;

jref void_box() {
    Class *c = load_boot_class("java/lang/Void");
    return Allocator::object(c);
}

jref byte_box(jbyte x) {
    Class *c = load_boot_class("java/lang/Byte");
    // public static Byte valueOf(byte b);
    Method *m = c->get_method("valueOf", "(B)Ljava/lang/Byte;");
    return execJavaR(m, {islot(x)});
}

jref bool_box(jbool x) {
    Class *c = load_boot_class("java/lang/Boolean");
    // public static Boolean valueOf(boolean b);
    Method *m = c->get_method("valueOf", "(Z)Ljava/lang/Boolean;");
    return execJavaR(m, {islot(x)});
}

jref char_box(jchar x) {
    Class *c = load_boot_class("java/lang/Character");
    // public static Character valueOf(char c);
    Method *m = c->get_method("valueOf", "(C)Ljava/lang/Character;");
    return execJavaR(m, {islot(x)});
}

jref short_box(jshort x) {
    Class *c = load_boot_class("java/lang/Short");
    // public static Short valueOf(short s);
    Method *m = c->get_method("valueOf", "(S)Ljava/lang/Short;");
    return execJavaR(m, {islot(x)});
}

jref int_box(jint x) {
    Class *c = load_boot_class("java/lang/Integer");
    // public static Integer valueOf(int i);
    Method *m = c->get_method("valueOf", "(I)Ljava/lang/Integer;");
    return execJavaR(m, {islot(x)});
}

jref float_box(jfloat x) {
    Class *c = load_boot_class("java/lang/Float");
    // public static Float valueOf(float f);
    Method *m = c->get_method("valueOf", "(F)Ljava/lang/Float;");
    return execJavaR(m, {fslot(x)});
}

jref long_box(jlong x) {
    Class *c = load_boot_class("java/lang/Long");
    // public static Long valueOf(long f);
    Method *m = c->get_method("valueOf", "(J)Ljava/lang/Long;");
    slot_t args[2];
    slot::set<jlong>(args, x);
    return execJavaR(m, args);
}

jref double_box(jdouble x) {
    Class *c = load_boot_class("java/lang/Double");
    // public static Double valueOf(double d);
    Method *m = c->get_method("valueOf", "(D)Ljava/lang/Double;");
    slot_t args[2];
    slot::set<jdouble>(args, x);
    return execJavaR(m, args);
}

TEST_CASE(test_box, {
    if (!std::holds_alternative<std::monostate>(void_box()->box_value())) {
        std::cout << "failed" << std::endl;
    }

    auto v = int_box(100)->box_value();
    if (std::holds_alternative<int>(v)) {
        auto x = std::get<jint>(v);
        if (x != 100)
            std::cout << "failed" << std::endl;
    }

    v = double_box(100.67)->box_value();
    if (std::holds_alternative<jdouble>(v)) {
        auto x = std::get<jdouble>(v);
        if (x != 100.67)
            std::cout << "failed" << std::endl;
    }
})