export module native;

import runtime;

export void init_native();

export void registry(const char *class_name, const char *method_name,
                     const char *method_descriptor, void (*method)(Frame *));

export void (* find_native(const char *class_name, const char *method_name, const char *method_descriptor))(Frame *);



