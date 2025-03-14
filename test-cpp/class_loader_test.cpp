#include "test.h"

using namespace std;

TEST_CASE(test_load_class)
    const char *class_names[2];
    class_names[0] = "boolean";
    class_names[1] = "java/lang/Class";

    for (const char *class_name: class_names) {
        Class *c = load_boot_class(class_name);
        printf("%p, %s, %s, %d\n", c, c->name,
               c->java_mirror->jvm_mirror->name,
               c == c->java_mirror->jvm_mirror);
    }
}

TEST_CASE(test_classloader1)
    const unordered_set<const Object *> &_loaders = getAllClassLoaders();
    printf("loaders count: %lld\n", _loaders.size());
    for (auto loader: _loaders) {
        if (loader == BOOT_CLASS_LOADER)
            printf("boot class loader\n");
        else
            printf("%s\n", loader->clazz->name);
    }

    auto bc = getAllBootClasses();
    printf("---\n\tboot classes count: %d\n", bc->size());
    for (auto &p: *bc) {
        printf("\t\t%s\n", p.first);
    }
}
