#include "test.h"

using namespace std;

void test_inject_field() {
// const char *class_names[] = {
//     "java/lang/Object",
//     "java/lang/Class",
//     "java/lang/Object", // 第二次注入 java/lang/Object
// };

// for (const char *class_name : class_names) {
//     Class *c = loadBootClass(class_name);
//     printf("%s\n", c->name);

//     // 因为 injectInstField 只能在 loaded 之后进行，
//     // 所以这里为了测试强制设置一下。
//     Class::State state = c->state;
//     c->state = Class::State::LOADED;
//     bool b1 = c->injectInstField("inject1", "C");
//     bool b2 = c->injectInstField("inject2", "I");
//     bool b3 = c->injectInstField("inject3", "J");
//     c->state = state;

//     printf("\t%d, %d, %d\n", b1, b2, b3);
//     printf("\t%s\n", c->toString().c_str());
// }
}
