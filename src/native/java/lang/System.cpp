module;
#include "../../../vmdef.h"

module native;

import slot;
import classfile;
import object;
import runtime;

using namespace std;

// private static native void setIn0(InputStream in);
void setIn0(Frame *f) {
    slot_t *args = f->lvars;
    auto stream = slot::get<jref>(args);

    Field *field = f->method->clazz->get_field("in");
    field->static_value.r = stream;
}

// private static native void setOut0(PrintStream out);
void setOut0(Frame *f) {
    slot_t *args = f->lvars;
    auto stream = slot::get<jref>(args);

    Field *field = f->method->clazz->get_field("out");
    field->static_value.r = stream;
}

// private static native void setErr0(PrintStream err);
void setErr0(Frame *f) {
    slot_t *args = f->lvars;
    auto stream = slot::get<jref>(args);

    Field *field = f->method->clazz->get_field("err");
    field->static_value.r = stream;
}

// public static native long currentTimeMillis();
// The current time as UTC milliseconds from the epoch(1970-1-1-00:00:00 UTC).
void currentTimeMillis(Frame *f) {
    // Get the current time point
    auto now = std::chrono::system_clock::now();
    // Calculate the duration from the epoch to the current time point
    auto duration = now.time_since_epoch();
    // Convert the duration to milliseconds
    auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
    f->pushl(milliseconds);
}

/*
 * 返回最准确的可用系统计时器的当前值，以毫微秒为单位。
 * 此方法只能用于测量已过的时间，与系统或钟表时间的其他任何时间概念无关。
 * 返回值表示从某一固定但任意的时间算起的毫微秒数（或许从以后算起，所以该值可能为负）。
 * 此方法提供毫微秒的精度，但不是必要的毫微秒的准确度。它对于值的更改频率没有作出保证。
 * 在取值范围大于约 292 年（263 毫微秒）的连续调用的不同点在于：由于数字溢出，将无法准确计算已过的时间。
 *
 * public static native long nanoTime();
 */
void nanoTime(Frame *f) {
    auto now = std::chrono::high_resolution_clock::now();
    f->pushl(now.time_since_epoch().count());
}

// public static native void arraycopy(Object src,  int srcPos,
//                                    Object dest, int destPos,
//                                    int length);
void arraycopy(Frame *f) {
    slot_t *args = f->lvars;
    auto src = slot::get<jref>(args++);
    auto src_pos = slot::get<jint>(args++);
    auto dest = slot::get<jref>(args++);
    auto dest_pos = slot::get<jint>(args++);
    auto length = slot::get<jint>(args);

    assert(dest->is_array_object());
    assert(src->is_array_object());
    array_copy(dest, dest_pos, src, src_pos, length);
}

// public static native int identityHashCode(Object x);
void identityHashCode(Frame *f) {
    slot_t *args = f->lvars;
    auto obj = slot::get<jref>(args);
    f->pushi((jint)(intptr_t)obj); // todo 实现错误。改成当前的时间如何。
}

// public static native String mapLibraryName(String libname);
void mapLibraryName(Frame *f) {
    slot_t *args = f->lvars;
    auto libname_obj = slot::get<jref>(args);
    auto libname = java_lang_String::to_utf8(libname_obj);

    string s;
    s.append(JNI_LIB_PREFIX).append(libname).append(JNI_LIB_SUFFIX);
    auto o = Allocator::string(s.c_str());
    f->pushr(o);
}

// private static native void registerNatives();
void java_lang_System_registerNatives(Frame *f) {
#undef R
#define R(method, method_descriptor) \
    registry("java/lang/System", #method, method_descriptor, method)

    R(setErr0, "(Ljava/io/PrintStream;)V");
    R(setIn0, "(Ljava/io/InputStream;)V");
    R(setOut0, "(Ljava/io/PrintStream;)V");
    R(arraycopy, "(Ljava/lang/Object;ILjava/lang/Object;II)V");
    R(currentTimeMillis, "()J");
    R(identityHashCode, "(Ljava/lang/Object;)I");
    //R(initProperties, "(Ljava/util/Properties;)Ljava/util/Properties;");
    R(nanoTime, "()J");
    R(mapLibraryName, "(Ljava/lang/String;)Ljava/lang/String;");
}