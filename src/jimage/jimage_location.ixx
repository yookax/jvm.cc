module;
#include <assert.h>
#include "../vmdef.h"

export module jimage_location;

import std.core;
import jimage_strings;

using namespace std;

// jdk/internal/jimage/ImageLocation.java

static const int ATTRIBUTE_END = 0;
static const int ATTRIBUTE_MODULE = 1;
static const int ATTRIBUTE_PARENT = 2;
static const int ATTRIBUTE_BASE = 3;
static const int ATTRIBUTE_EXTENSION = 4;
static const int ATTRIBUTE_OFFSET = 5;
static const int ATTRIBUTE_COMPRESSED = 6;
static const int ATTRIBUTE_UNCOMPRESSED = 7;
static const int ATTRIBUTE_COUNT = 8;

export struct JImageLocation {
    jlong attributes[ATTRIBUTE_COUNT] = { 0 };

    jlong get_offset() {
        return attributes[ATTRIBUTE_OFFSET];
    }

    jlong get_uncompressed() {
        return attributes[ATTRIBUTE_UNCOMPRESSED];
    }

    string get_full_name(bool modules_prefix, const char *strings) {
        ostringstream oss;
        if (attributes[ATTRIBUTE_MODULE] != 0) {
            if (modules_prefix) {
                oss << "/modules";
            }
            auto index = attributes[ATTRIBUTE_MODULE];
            oss << "/" << strings + index << "/";
        }

        if (attributes[ATTRIBUTE_PARENT] != 0) {
            auto index = attributes[ATTRIBUTE_PARENT];
            oss << strings + index << "/";
        }

        oss << strings + attributes[ATTRIBUTE_BASE];

        if (attributes[ATTRIBUTE_EXTENSION] != 0) {
            auto index = attributes[ATTRIBUTE_EXTENSION];
            oss << "." << strings + index;
        }

        return oss.str();
    }

    bool verify(const char8_t *path, const char *strings) {
        string full_name = get_full_name(false, strings);
        return strcmp((char *)path, full_name.c_str()) == 0;
    }

    string to_str(const char *strings) {
        ostringstream oss;
        if (attributes[ATTRIBUTE_END] != 0) {

        }
        oss << "end: " << attributes[ATTRIBUTE_END] << endl;
        if (attributes[ATTRIBUTE_MODULE] != 0) {
            auto index = attributes[ATTRIBUTE_MODULE];
            oss << "module: " << strings + index << endl;
        }
        if (attributes[ATTRIBUTE_PARENT] != 0) {
            auto index = attributes[ATTRIBUTE_PARENT];
            oss << "parent: " << strings + index << endl;
        }
        if (attributes[ATTRIBUTE_BASE] != 0) {
            auto index = attributes[ATTRIBUTE_BASE];
            oss << "base: " << strings + index << endl;
        }
        //oss << "base: " << attributes[ATTRIBUTE_BASE] << endl;
        if (attributes[ATTRIBUTE_EXTENSION] != 0) {
            auto index = attributes[ATTRIBUTE_EXTENSION];
            oss << "extension: " << strings + index << endl;
        }
        oss << "offset: " << attributes[ATTRIBUTE_OFFSET] << endl;
        oss << "compressed: " << attributes[ATTRIBUTE_COMPRESSED] << endl;
        oss << "compressed: " << attributes[ATTRIBUTE_UNCOMPRESSED] << endl;
        return oss.str();
    }
};

export JImageLocation decompress(const uint8_t *bytes, size_t offset, size_t bytes_len) {
    JImageLocation location;
    while (offset < bytes_len) {
        uint8_t data = bytes[offset++] & 0xFF;
        if (data <= 0x7) { // ATTRIBUTE_END
            break;
        }
        int kind = data >> 3;
        if (ATTRIBUTE_COUNT <= kind) {
            panic("");
            //throw new InternalError("Invalid jimage attribute kind: " + kind); todo
        }

        int length = (data & 0x7) + 1;
        long value = 0;
        for (int j = 0; j < length; j++) {
            value <<= 8;
            if (offset >= bytes_len) {
                panic("");
                //throw new InternalError("Missing jimage attribute data");todo
            }
            value |= bytes[offset++] & 0xFF;
        }

        location.attributes[kind] = value;
    }
    return location;
}
