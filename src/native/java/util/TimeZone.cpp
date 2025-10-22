module;
#include "../../../vmdef.h"

module native;

import std;
import object;
import runtime;

using namespace std;

/*
 * Gets the platform defined TimeZone ID.
 */
// private static native String getSystemTimeZoneID(String javaHome);
void getSystemTimeZoneID(Frame *f) {
//    slot_t *args = f->lvars;
//    auto home = slot::get<jref>(args);

    // 获取当前时间
    std::time_t now = std::time(nullptr);
    std::tm *local_time = std::localtime(&now);

    // 尝试获取时区名称
    char tzname[128]; // I think it's big enough
    if (std::strftime(tzname, sizeof(tzname), "%Z", local_time) > 0) {
        auto s = Allocator::string(tzname);
        f->pushr(s);
    } else {
        f->pushr(nullptr);
    }
}

/*
 * Gets the custom time zone ID based on the GMT offset of the
 * platform. (e.g., "GMT+08:00")
 */
// private static native String getSystemGMTOffsetID();
void getSystemGMTOffsetID(Frame *f) {
    unimplemented
}


void java_util_TimeZone_registerNatives() {
#undef R
#define R(method, method_descriptor) \
    registry("java/util/TimeZone", #method, method_descriptor, method)

    R(getSystemTimeZoneID, "(Ljava/lang/String;)Ljava/lang/String;");
}