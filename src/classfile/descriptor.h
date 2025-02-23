#ifndef CABIN_DESCRIPTOR_H
#define CABIN_DESCRIPTOR_H

#include <string>
#include "../cabin.h"

int numSlotsInMethodDescriptor(const char *method_descriptor);

// @b: include
// @e：exclude
// eg. I[BLjava/lang/String;ZZ, return 5.
int numEltsInDescriptor(const char *b, const char *e);

int numEltsInMethodDescriptor(const char *method_descriptor);

std::pair<jarrRef /*ptypes*/, jclsRef /*rtype*/>
parseMethodDescriptor(const char *desc, jref loader);

std::string unparseMethodDescriptor(jarrRef ptypes /* ClassObject *[] */, jclsRef rtype);

// @method_type: Object of java.lang.invoke.MethodType
std::string unparseMethodDescriptor(jref method_type);

#endif //CABIN_DESCRIPTOR_H
