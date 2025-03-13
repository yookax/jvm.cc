#include "test.h"

using namespace std;

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

TEST_CASE(test_method_descriptor)
    for (const char *d: method_descriptors) {
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
}

