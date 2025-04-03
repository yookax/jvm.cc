module;
#include <cassert>

export module image_file;

import std.core;
import bytes_reader;
import sysinfo;

// jdk/internal/jimage/ImageHeader
class JImageHeader {
    uint32_t magic; // Image file marker
    uint16_t major_version; // Image file major version number
    uint16_t minor_version;
    uint32_t flags; // Image file flags
    uint32_t resource_count; // Number of resources in file
    uint32_t table_length; // Number of slots in index tables
    uint32_t locations_size; // Number of bytes in attribute table
    uint32_t strings_size; // Number of bytes in string table

public:
    void read(BytesReader &r) {
        auto magic = r.readu4();
        auto major_version = r.readu2();
        auto minor_version = r.readu2();
        auto flags = r.readu4();
        auto resource_count = r.readu4();
        auto table_length = r.readu4();
        auto locations_size = r.readu4();
        auto strings_size = r.readu4();
    }
};

auto path = "C:/Java/jdk-24/lib/modules";

class JImageFile {
    BytesReader *reader;

    JImageHeader header;
public:
    JImageFile(const char *jimage_file_path) {
        auto size = get_file_size(jimage_file_path);
        auto mapping = mem_mapping(jimage_file_path);
        reader = new BytesReader((const uint8_t *) mapping, size, std::endian::native);
    }

    void read() {
        header.read(*reader);
    }
};