module;
#include <cassert>
#include "../cabin.h"

module classfile;

import std.core;
import vmstd;
import object;
import invoke;
import class_loader;

using namespace std;

// @b: include
// @e：exclude
// eg. I[BLjava/lang/String;JD, return 7.
static int numSlotsInDescriptor(const char *b, const char *e) {
    assert(b != nullptr && e != nullptr);
    assert(b <= e);

    int count = 0;
    for (; b < e; b++, count++) {
        if (*b == 'D' || *b == 'J'/* long */) {
            count++;
        } else  {  
            if(*b == '[')
                while(*++b == '[');
            if(*b == 'L')
                while(*++b != ';');
        }
    }
    return count;
}

int numSlotsInMethodDescriptor(const char *method_descriptor) {
    assert(method_descriptor != nullptr && method_descriptor[0] == '(');

    const char *b = method_descriptor + 1; // jump '('
    const char *e = strchr(method_descriptor, ')');
    return numSlotsInDescriptor(b, e);
}

// @b: include
// @e：exclude
// eg. Ljava/lang/String;
static Object *convertDescElement2ClassObject(const char *&b, const char *e, jref loader) {
    assert(b != nullptr && e != nullptr);

    if (*b == 'L') { // reference
        const char *t = strchr(b, ';');
        if (t == nullptr || t >= e) {
            goto error;
        }

        b++; // jump 'L'
        auto c = loadClass(loader, string(b, t - b).c_str());
        b = t + 1;
        return c->java_mirror;
    }

    if (*b == '[') { // array reference, 描述符形如 [B 或 [[Ljava/lang/String; 的形式
        const char *t = b;
        while (*(++t) == '[');
        if (!is_prim_descriptor(*t)) {
            t = strchr(t, ';');
            if (t == nullptr || t >= e) {
                goto error;
            }
        }

        t++;
        auto c = load_array_class(loader, string(b, t - b).c_str());
        b = t;
        return c->java_mirror;
    }

    if (is_prim_descriptor(*b)) { // prim type
        const char *class_name = get_prim_class_name(*b);
        b++;
        return load_boot_class(class_name)->java_mirror;
    }

error:
    throw java_lang_UnknownError(FILE_LINE_STR);
}

int numEltsInDescriptor(const char *b, const char *e) {
    assert(b != nullptr && e != nullptr);

    int no_params;
    b--;
    for(no_params = 0; ++b < e; no_params++) {
        if(*b == '[')
            while(*++b == '[');
        if(*b == 'L')
            while(*++b != ';');
    }

    return no_params;
}

int numEltsInMethodDescriptor(const char *method_descriptor) {
    assert(method_descriptor != nullptr && method_descriptor[0] == '(');

    const char *b = method_descriptor + 1; // jump '('
    const char *e = strchr(method_descriptor, ')');
    return numEltsInDescriptor(b, e);
}

//int numElementsInDescriptor(const char *descriptor)
//{
//    assert(descriptor != nullptr);
//    return numElementsInDescriptor(descriptor, descriptor + strlen(descriptor));
//}

// @b: include
// @e：exclude
// eg. I[BLjava/lang/String;ZZ
static jarrRef convertDesc2ClassObjectArray(const char *b, const char *e, jref loader) {
    int num = numEltsInDescriptor(b, e);
    jarrRef types = Allocator::class_array(num);

    for (int i = 0; b < e; i++) {
        Object *co = convertDescElement2ClassObject(b, e, loader);
        assert(i < num);
        types->setRefElt(i, co);
    }

    return types;
}

pair<jarrRef, jclsRef> parseMethodDescriptor(const char *desc, jref loader) {
    assert(desc != nullptr);

    const char *e = strchr(desc, ')');
    if (e == nullptr || *desc != '(') {
        // throw java_lang_UnknownError(); // todo
    }

    jarrRef ptypes = convertDesc2ClassObjectArray(desc + 1, e, loader);
    e++; // jump ')'
    jclsRef rtype = convertDescElement2ClassObject(e, e + strlen(e), loader);
    return make_pair(ptypes, rtype);
}

static string convertTypeToDesc(Class *type) {
    assert(type != nullptr);

    if (type->is_prim_class()) {
        return get_prim_descriptor_by_class_name(type->name);
    }

    if (type->is_array_class()) {
        return type->name;
    }

    // 普通类
    ostringstream oss;
    oss << 'L';
    oss << type->name;
    oss << ';';
    return oss.str();
}

string unparseMethodDescriptor(jarrRef ptypes /*Class *[]*/, jclsRef rtype) {
    ostringstream oss;

    if (ptypes == nullptr) { // no argument
        oss << "()";
    } else {
        oss << "(";
        for (int i = 0; i < ptypes->arr_len; i++) {
            auto co = ptypes->getElt<jclsRef>(i);
            oss << convertTypeToDesc(co->jvm_mirror);        
        }
        oss << ")";
    }

    if (rtype == nullptr) { // no return value
        oss << "V";
    } else {
        oss << convertTypeToDesc(rtype->jvm_mirror);
    }

    return oss.str();
}

string unparseMethodDescriptor(jref method_type) {
    assert(method_type != nullptr);

    // private final Class<?>[] ptypes;
    auto ptypes = method_type->get_field_value<jref>("ptypes", "[Ljava/lang/Class;");
    // private final Class<?> rtype;
    auto rtype = method_type->get_field_value<jref>("rtype", "Ljava/lang/Class;");

    return unparseMethodDescriptor(ptypes, rtype);
}

// -----------------------------------------------------------------------------

static const char *method_descriptors[] = {
        "()V",
        "(I)V",
        "(B)C",
        "(Ljava/lang/Integer;)V",
        "(Ljava/lang/Object;[[BLjava/lang/Integer;[Ljava/lang/Object;)V",
        "(II[Ljava/lang/String;)Ljava/lang/Integer;",
        "([Ljava/io/File;)Ljava/lang/Object;",
        "(J[Ljava/io/File;Ljava/io/File;D)Ljava/lang/Object;",
        "([[[Ljava/lang/Double;)[[Ljava/lang/Object;",
        "(ZBSIJFD)[[Ljava/lang/String;",
        "(ZZZZZZZZZZZZZZZZ)Z",
};

TEST_CASE(test_method_descriptor, {
    for (const char *d : method_descriptors) {
        printf("%s\n\tparameters number: %d\n", d, numEltsInMethodDescriptor(d));
        printf("\tparameter slots number: %d\n\tparameter types: ", numSlotsInMethodDescriptor(d));

        auto p = parseMethodDescriptor(d, BOOT_CLASS_LOADER);
        /*ptypes*/
        for (int i = 0; i < p.first->arr_len; i++) {
            jref o = p.first->getElt<jref>(i);
            printf("%s, ", o->jvm_mirror->name);
        }
        printf("\n\treturn type: %s\n", p.second->jvm_mirror->name); /*rtype*/
        printf("\tunparse: %s\n\t-----\n\tparameter types: ", unparseMethodDescriptor(p.first, p.second).c_str());

        jref mt = findMethodType(d, BOOT_CLASS_LOADER);
        jarrRef ptypes = mt->get_field_value<jref>("ptypes", "[Ljava/lang/Class;");
        for (int i = 0; i < ptypes->arr_len; i++) {
            jref o = ptypes->getElt<jref>(i);
            printf("%s, ", o->jvm_mirror->name);
        }

        printf("\n\tunparse: %s\n", unparseMethodDescriptor(mt).c_str());
    }

    printf("---\nunparse(NULL, NULL): %s\n", unparseMethodDescriptor(nullptr, nullptr).c_str());
})

