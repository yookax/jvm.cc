module;
#include <cassert>
#include "vmdef.h"

module vmstd;

import std.core;
import object;
import class_loader;

using namespace slot;

Object *JavaException::get_excep() {
    if (g_vm_initing) {
        fprintf(stderr, "Exception occurred while VM initialising.\n");
        if(!msg.empty())
            fprintf(stderr, "%s: %s\n", excep_class_name, msg.c_str());
        else
            fprintf(stderr, "%s\n", excep_class_name);
        exit(1); // todo
    }

    if (excep == nullptr) {
        Class *ec = load_boot_class(excep_class_name);
        init_class(ec);
        excep = Allocator::object(ec);
        if (msg.empty()) {
            execJava(ec->get_constructor("()V"), { rslot(excep) });
        } else {
            Method *constructor = ec->get_constructor("(Ljava/lang/String;)V");
            execJava(constructor, { rslot(excep), rslot(Allocator::string(msg.c_str())) });
        }
    }

    assert(excep != nullptr);
    return excep;
}

void print_stack_trace(Object *e) {
    assert(e != nullptr);
    assert(e->clazz->is_subclass_of(load_boot_class("java/lang/Throwable")));

    // private String detailMessage;
    jstrRef msg = e->get_field_value<jref>("detailMessage", "Ljava/lang/String;");
    printf("%s: %s\n", e->clazz->name, msg != nullptr ? java_lang_String::to_utf8(msg) : "null");

    // [Ljava/lang/Object;
    jref backtrace = e->get_field_value<jref>("backtrace", "Ljava/lang/Object;");
    if (backtrace == nullptr)
        return;  // todo
    for (int i = 0; i < backtrace->arr_len; i++) {
        jref element = backtrace->getElt<jref>(i); // java.lang.StackTraceElement

        // private String declaringClass;
        // private String methodName;
        // private String fileName;
        // private int    lineNumber;
        jstrRef declaring_class = element->get_field_value<jref>("declaringClass", "Ljava/lang/String;");
        jstrRef method_name = element->get_field_value<jref>("methodName", "Ljava/lang/String;");
        jstrRef file_name = element->get_field_value<jref>("fileName", "Ljava/lang/String;");
        jint line_number = element->get_field_value<jint>("lineNumber");

        printf("\tat %s.%s(%s:%d)\n",
               java_lang_String::to_utf8(declaring_class),
               java_lang_String::to_utf8(method_name),
               file_name ? java_lang_String::to_utf8(file_name) : "(Unknown Source)",
               line_number);
    }
}