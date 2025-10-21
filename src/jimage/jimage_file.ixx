module;
#include <assert.h>
#include "../vmdef.h"

export module jimage_file;

export import jimage_strings;
export import jimage_location;
import std.core;
import bytes_reader;
import sysinfo;

using namespace std;

// Image files are an alternate file format for storing classes and resources. The
// goal is to supply file access which is faster and smaller than the jar format.
// It should be noted that unlike jars, information stored in an image is in native
// endian format. This allows the image to be mapped into memory without endian
// translation.  This also means that images are platform dependent.
//
// Image files are structured as three sections;
//
//         +-----------+
//         |  Header   |
//         +-----------+
//         |           |
//         |   Index   |
//         |           |
//         +-----------+
//         |           |
//         |           |
//         | Resources |
//         |           |
//         |           |
//         +-----------+
//
// The header contains information related to identification and description of
// contents.
//
//         +-------------------------+
//         |   Magic (0xCAFEDADA)    |
//         +------------+------------+
//         | Major Vers | Minor Vers |
//         +------------+------------+
//         |          Flags          |
//         +-------------------------+
//         |      Resource Count     |
//         +-------------------------+
//         |       Table Length      |
//         +-------------------------+
//         |      Attributes Size    |
//         +-------------------------+
//         |       Strings Size      |
//         +-------------------------+
//
// Magic - means of identifying validity of the file.  This avoids requiring a
//         special file extension.
// Major vers, minor vers - differences in version numbers indicate structural
//                          changes in the image.
// Flags - various image wide flags (future).
// Resource count - number of resources in the file.
// Table length - the length of lookup tables used in the index.
// Attributes size - number of bytes in the region used to store location attribute streams.
// Strings size - the size of the region used to store strings used by the
//                index and meta data.
//
// The index contains information related to resource lookup. The algorithm
// used for lookup is "A Practical Minimal Perfect Hashing Method"
// (http://homepages.dcc.ufmg.br/~nivio/papers/wea05.pdf). Given a path string
// in the form /<module>/<package>/<base>.<extension>  return the resource location
// information;
//
//     redirectIndex = hash(path, DEFAULT_SEED) % table_length;
//     redirect = redirectTable[redirectIndex];
//     if (redirect == 0) return not found;
//     locationIndex = redirect < 0 ? -1 - redirect : hash(path, redirect) % table_length;
//     location = locationTable[locationIndex];
//     if (!verify(location, path)) return not found;
//     return location;
//
// Note: The hash function takes an initial seed value.  A different seed value
// usually returns a different result for strings that would otherwise collide with
// other seeds. The verify function guarantees the found resource location is
// indeed the resource we are looking for.
//
// The following is the format of the index;
//
//         +-------------------+
//         |   Redirect Table  |
//         +-------------------+
//         | Attribute Offsets |
//         +-------------------+
//         |   Attribute Data  |
//         +-------------------+
//         |      Strings      |
//         +-------------------+
//
// Redirect Table - Array of 32-bit signed values representing actions that
//                  should take place for hashed strings that map to that
//                  value.  Negative values indicate no hash collision and can be
//                  quickly converted to indices into attribute offsets.  Positive
//                  values represent a new seed for hashing an index into attribute
//                  offsets.  Zero indicates not found.
// Attribute Offsets - Array of 32-bit unsigned values representing offsets into
//                     attribute data.  Attribute offsets can be iterated to do a
//                     full survey of resources in the image.  Offset of zero
//                     indicates no attributes.
// Attribute Data - Bytes representing compact attribute data for locations. (See
//                  comments in ImageLocation.)
// Strings - Collection of zero terminated UTF-8 strings used by the index and
//           image meta data.  Each string is accessed by offset.  Each string is
//           unique.  Offset zero is reserved for the empty string.
//
// Note that the memory mapped index assumes 32 bit alignment of each component
// in the index.
//
// Endianness of an image.
// An image booted by hotspot is always in native endian.  However, it is possible
// to read (by the JDK) in alternate endian format.  Primarily, this is during
// cross platform scenarios.  Ex, where javac needs to read an embedded image
// to access classes for crossing compilation.
//

export class JImageFile {
    BytesReader *reader;
    MemMapping mem_mapping;
public:
    // jdk/internal/jimage/ImageHeader.java
    struct Header {
        static const uint32_t MAGIC = 0xCAFEDADA;
        static const uint16_t MAJOR_VERSION = 1;
        static const uint16_t MINOR_VERSION = 0;

        uint32_t flags;          // Image file flags
        uint32_t resource_count; // Number of resources in file
        uint32_t table_length;   // Number of slots in index tables
        uint32_t locations_size; // Number of bytes in attribute table
        uint32_t strings_size;   // Number of bytes in string table

        void read_from(BytesReader &r) {
            auto magic = r.readu4();
            assert(magic == MAGIC);
            auto version = r.readu4();
            auto major_version = version >> 16;
            auto minor_version = version & 0xFFFF;
            assert(major_version == MAJOR_VERSION);
            assert(minor_version == MINOR_VERSION);
            flags = r.readu4();
            resource_count = r.readu4();
            table_length = r.readu4();
            locations_size = r.readu4();
            strings_size = r.readu4();
        }

        void print() {
            std::print(cout,
                         "MAGIC: {:X}\n"
                          "version: {}.{}\n"
                          "flags: {}\n"
                          "resource count: {}\n"
                          "table length: {}\n"
                          "locations size: {}\n"
                          "strings size: {}\n",
                          MAGIC, MAJOR_VERSION, MINOR_VERSION,
                          flags, resource_count, table_length,
                          locations_size, strings_size);
        }
    } header;

    struct Index {
        vector<int32_t> redirect_table;
        vector<uint32_t> attribute_offsets;
        const uint8_t *locations;
        const char *strings;

        void read_from(BytesReader &r, const Header &h) {
            for (uint32_t i = 0; i < h.table_length; i++) {
                redirect_table.push_back(r.reads4());
            }
            for (uint32_t i = 0; i < h.table_length; i++) {
                attribute_offsets.push_back(r.readu4());
            }
            locations = r.currPos();
            r.skip(h.locations_size);
            strings = (char *) r.currPos();
            r.skip(h.strings_size);
        }

        void print() {

        }
    } index;

    const uint8_t *resources;

    JImageFile(const char *jimage_file_path) try: mem_mapping(jimage_file_path) {
        auto size = getFileSize(jimage_file_path);
        reader = new BytesReader((const uint8_t *) mem_mapping.address, size, std::endian::native);
        header.read_from(*reader);
        index.read_from(*reader, header);
        resources = reader->currPos();
    } catch(...) {
        panic("ERROR: Failed to create a memory mapping.\n");
    }

//     redirectIndex = hash(path, DEFAULT_SEED) % table_length;
//     redirect = redirectTable[redirectIndex];
//     if (redirect == 0) return not found;
//     locationIndex = redirect < 0 ? -1 - redirect : hash(path, redirect) % table_length;
//     location = locationTable[locationIndex];
//     if (!verify(location, path)) return not found;
//     return location;
    optional<JImageLocation> find_location(const char8_t *path) {
        u8string s(path);
        auto hash = hash_code(s);
        auto redirect_index = hash % header.table_length;
        auto redirect = index.redirect_table[redirect_index];
        if (redirect == 0) {
            // not found;
            return nullopt;
        }
        auto location_index = redirect < 0 ? -1 - redirect : hash_code(s, redirect) % header.table_length;
        auto offset = index.attribute_offsets[location_index];
        JImageLocation loc = decompress(index.locations, offset, header.locations_size);
        if (loc.verify(path, index.strings))
            return loc;
        else
            return nullopt;
    }

    optional<pair<const uint8_t *, size_t>> get_resource(const char8_t *path) {
        optional<JImageLocation> loc = find_location(path);
        if (!loc.has_value()) {
            return nullopt;
        }

        auto x = loc.value();
        return make_pair(resources + x.get_offset(), x.get_uncompressed());
    }

    void print() {
        header.print();
        index.print();
    }
};

