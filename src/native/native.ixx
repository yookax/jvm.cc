export module native;

//#include <string>
//#include <unordered_map>
import std.core;

using namespace std;

class Frame;

export void init_native();

export void registry(const char *class_name, const char *method_name,
                     const char *method_descriptor, void (*method)(Frame *));
//
//export void registry(const char *key, void (*method)(Frame *));

export void (* find_native(const char *class_name, const char *method_name, const char *method_descriptor))(Frame *);



