module;
#include <cassert>

export module raw_classfile;

import std.core;
import bytes_reader;
import encoding;
import constants;
import convert;

using namespace std;

union ConstantValue {
    uint16_t index;
    int32_t i32;
    int64_t i64;
    float f32;
    double f64;

    u8string *str;

    struct {
        uint16_t name_index;
        uint16_t descriptor_index;
    } name_and_type;

    struct {
        uint16_t class_index;
        uint16_t name_and_type_index;
    } field, method, interface_method;

    struct {
        uint16_t bootstrap_method_attr_index;
        uint16_t name_and_type_index;
    } dynamic, invoke_dynamic;

    struct {
        uint8_t reference_kind;
        uint16_t reference_index;
    } method_handle;

    explicit ConstantValue(uint16_t x): index(x) { }
    explicit ConstantValue(int32_t x): i32(x) { }
    explicit ConstantValue(int64_t x): i64(x) { }
    explicit ConstantValue(float x): f32(x) { }
    explicit ConstantValue(double x): f64(x) { }
    explicit ConstantValue(u8string *s): str(s) { }

    ConstantValue(uint8_t x, uint16_t y) {
        method_handle.reference_kind = x;
        method_handle.reference_index = y;
    }

    ConstantValue(uint16_t x, uint16_t y) {
        name_and_type.name_index = x;
        name_and_type.descriptor_index = y;
    }
};

struct RawAttribute {
    uint16_t attribute_name_index;
    uint32_t attribute_length;
    const uint8_t *info;

    explicit RawAttribute(BytesReader &r) {
        attribute_name_index = r.readu2();
        attribute_length = r.readu4();
        info = r.curr_pos();
        r.skip(attribute_length);
    }
};

struct RawField {
    uint16_t access_flags;
    u8string *name;
    u8string *descriptor;
    vector<RawAttribute> attributes;

    explicit RawField(BytesReader &r, vector<pair<uint8_t, ConstantValue>> &cp) {
        access_flags = r.readu2();

        auto name_index = r.readu2();
        auto &[tag, value] = cp.at(name_index);
        assert(tag == JVM_CONSTANT_Utf8);
        name = value.str;

        auto descriptor_index = r.readu2();
        auto &[tag1, value1] = cp.at(descriptor_index);
        assert(tag1 == JVM_CONSTANT_Utf8);
        descriptor = value1.str;

        uint16_t attributes_count = r.readu2();
        for (uint16_t i = 0; i < attributes_count; i++) {
            attributes.emplace_back(r);
        }
    }

    string to_str() const {
        ostringstream oss;
        oss << (char *) name->c_str() << "~" << (char *) descriptor->c_str() << endl;
        return oss.str();
    }
};

using RawMethod = RawField;

export struct RawClassfile {
    uint8_t *content = nullptr;
    ~RawClassfile() { delete[] content; }

    uint32_t magic;
    uint16_t minor_version;
    uint16_t major_version;
    vector<pair<uint8_t, ConstantValue>> constant_pool;
    uint16_t access_flags;
    u8string *this_class;
    u8string *super_class;
    vector<uint16_t> interfaces;
    vector<RawField> fields;
    vector<RawMethod> methods;
    vector<RawAttribute> attributes;
    
    RawClassfile(const char *classfile_path) {
        if (classfile_path == nullptr) {
            throw "";
        }
        ifstream file(classfile_path, ios::binary);
        if (!file) {
            throw "Failed to open the file.";
        }

        file.seekg(0, ios::end);
        streamsize size = file.tellg();
        if (size == -1) {
            file.close();
            throw "Failed to get the file size.";
        }
        file.seekg(0, ios::beg);

        content = new uint8_t[size];
        file.read((char *)content, size);

        if (file.gcount() != size) {
            file.close();
            throw "An error occurred while reading the file. The file was not read completely.";
        }

        file.close();

//        cout << content << endl;
        BytesReader r(content, size, endian::big);

        magic = r.readu4();
        if (magic != 0xCAFEBABE) {
            throw "Not a legal Java bytecode file.";
        }
        minor_version = r.readu2();
        major_version = r.readu2();

        // ------------------------------ constant pool ---------------------------------

        auto constant_pool_count = r.readu2();
        constant_pool.reserve(constant_pool_count);
        // constant pool 从 1 开始计数，第0位无效
        constant_pool.emplace_back(JVM_CONSTANT_Invalid,(int32_t) 0);
        for (uint16_t i = 1; i < constant_pool_count; i++) {
            auto tag = r.readu1();
            switch (tag) {
                case JVM_CONSTANT_Class:
                case JVM_CONSTANT_String:
                case JVM_CONSTANT_MethodType:
                case JVM_CONSTANT_Module:
                case JVM_CONSTANT_Package:
                    constant_pool.emplace_back(tag, r.readu2());
                    break;
                case JVM_CONSTANT_NameAndType:
                case JVM_CONSTANT_Fieldref:
                case JVM_CONSTANT_Methodref:
                case JVM_CONSTANT_InterfaceMethodref:
                case JVM_CONSTANT_Dynamic:
                case JVM_CONSTANT_InvokeDynamic: {
                    auto x = r.readu2();
                    auto y = r.readu2();
                    constant_pool.emplace_back(tag, ConstantValue(x, y));
                    break;
                }
                case JVM_CONSTANT_Integer: {
                    uint8_t bytes[4];
                    r.read_bytes(bytes, 4);
                    constant_pool.emplace_back(tag, bytes_to_int32(bytes, endian::big));
                    break;
                }
                case JVM_CONSTANT_Float: {
                    uint8_t bytes[4];
                    r.read_bytes(bytes, 4);
                    constant_pool.emplace_back(tag, bytes_to_float(bytes, endian::big));
                    break;
                }
                case JVM_CONSTANT_Long: {
                    uint8_t bytes[8];
                    r.read_bytes(bytes, 8);
                    constant_pool.emplace_back(tag, bytes_to_int64(bytes, endian::big));
                    constant_pool.emplace_back(JVM_CONSTANT_Placeholder, 0);
                    break;
                }
                case JVM_CONSTANT_Double: {
                    uint8_t bytes[8];
                    r.read_bytes(bytes, 8);
                    constant_pool.emplace_back(tag, bytes_to_double(bytes, endian::big));
                    constant_pool.emplace_back(JVM_CONSTANT_Placeholder, 0);
                    break;
                }
                case JVM_CONSTANT_Utf8: {
                    auto utf8_len = r.readu2();
                    constant_pool.emplace_back(tag, mutf8_to_new_utf8(r.curr_pos(), utf8_len));
                    r.skip(utf8_len);
                    break;
                }
                case JVM_CONSTANT_MethodHandle: {
                    auto x = r.readu1();
                    auto y = r.readu2();
                    constant_pool.emplace_back(tag, ConstantValue(x, y));
                    break;
                }
                default:
                    std::cerr << "bad constant tag: " << (int) tag << endl;
                    throw "";
            }
        }

        access_flags = r.readu2();

        auto this_class_index = r.readu2();
        auto &[tag, value] = constant_pool.at(this_class_index);
        assert(tag == JVM_CONSTANT_Class);
        auto &[tag1, value1] = constant_pool.at(value.index);
        assert(tag1 == JVM_CONSTANT_Utf8);
        this_class = value1.str;

        auto super_class_index = r.readu2();
        if (super_class_index == 0) {
            super_class = nullptr;
        } else {
            auto &[tag2, value2] = constant_pool.at(super_class_index);
            assert(tag2 == JVM_CONSTANT_Class);
            auto &[tag3, value3] = constant_pool.at(value2.index);
            assert(tag3 == JVM_CONSTANT_Utf8);
            super_class = value3.str;
        }

        // ------------------------------ interfaces count -----------------------------

        auto interfaces_count = r.readu2();
        for (uint16_t i = 0; i < interfaces_count; i++) {
            interfaces.push_back(r.readu2());
        }

        // ----------------------------------- fields ----------------------------------

        auto fields_count = r.readu2();
        for (uint16_t i = 0; i < fields_count; i++) {
            fields.emplace_back(r, constant_pool);
        }

        // ----------------------------------- methods ----------------------------------

        auto methods_count = r.readu2();
        for (uint16_t i = 0; i < methods_count; i++) {
            methods.emplace_back(r, constant_pool);
        }

        // ---------------------------------- attributes --------------------------------

        auto attributes_count = r.readu2();
        for (uint16_t i = 0; i < attributes_count; i++) {
            attributes.emplace_back(r);
        }
    }

    string to_str() {
        ostringstream oss;
        oss << "magic: 0xCAFEBABE" << endl;
        oss << "minor version: " << minor_version << endl;
        oss << "major version: " << major_version << endl;
        oss << "constant pool: " << constant_pool.size() << endl;
//        for (auto &x: constant_pool) {
//            if (x.first == JVM_CONSTANT_Utf8) {
//                cout << (char *)x.second.str->c_str() << endl;
//            }
//        }
        oss << "access flags: " << access_flags << endl;
        oss << "this class: " << (char *) this_class->c_str() << endl;
        oss << "super class: " << (char *) super_class->c_str() << endl;
        oss << "interfaces: " << interfaces.size() << endl;
        oss << "fields: " << fields.size() << endl;
        for (auto &f: fields) {
            oss << "    " << f.to_str();
        }
        oss << "methods: " << methods.size() << endl;
        for (auto &m: methods) {
            oss << "    " << m.to_str();
        }
        oss << "attributes: " << attributes.size() << endl;
        return oss.str();
    }
};