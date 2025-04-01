//module;
//#include <cstdint>

export module image_file;

import std.core;

// jdk/internal/jimage/ImageHeader
class image_header {
    uint32_t magic; // Image file marker
    uint16_t major_version; // Image file major version number
    uint16_t minor_version;
    uint32_t flags; // Image file flags
    uint32_t resource_count; // Number of resources in file
    uint32_t table_length; // Number of slots in index tables
    uint32_t locations_size; // Number of bytes in attribute table
    uint32_t strings_size; // Number of bytes in string table

public:
    void read(uint8_t *data) {

    }
};
