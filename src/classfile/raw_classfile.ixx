export module raw_classfile;

import std.core;
import bytes_reader;
import constants;
import convert;

using namespace std;

union ConstantValue {
    uint16_t utf8_index;
    int32_t i32;
    int64_t i64;
    float f32;
    double f64;
    uint8_t *p;

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

    explicit ConstantValue(uint16_t x): utf8_index(x) { }
    explicit ConstantValue(int32_t x): i32(x) { }
    explicit ConstantValue(int64_t x): i64(x) { }
    explicit ConstantValue(float x): f32(x) { }
    explicit ConstantValue(double x): f64(x) { }
    explicit ConstantValue(uint8_t *x): p(x) { }

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
    uint8_t info[0];

    explicit RawAttribute(BytesReader &r) {
        attribute_name_index = r.readu2();
        attribute_length = r.readu4();
        r.skip(attribute_length);
    }
};

struct RawField {
    uint16_t access_flags;
    uint16_t name_index;
    uint16_t descriptor_index;
    vector<RawAttribute> attributes;

    explicit RawField(BytesReader &r) {
        access_flags = r.readu2();
        name_index = r.readu2();
        descriptor_index = r.readu2();
        uint16_t attributes_count = r.readu2();
        for (uint16_t i = 0; i < attributes_count; i++) {
            attributes.emplace_back(r);
        }
    }
};

using RawMethod = RawField;

export struct RawClassfile {
    uint32_t magic;
    uint16_t minor_version;
    uint16_t major_version;
    vector<pair<uint8_t, ConstantValue>> constant_pool;
    uint16_t access_flags;
    uint16_t this_class;
    uint16_t super_class;
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

        auto content = new uint8_t[size];
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
        // constant pool 从 1 开始计数，第0位无效
        constant_pool.emplace_back(JVM_CONSTANT_Invalid, 0);
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
                    auto buf = new uint8_t[utf8_len + 1];
                    r.read_bytes(buf, utf8_len);
                    buf[utf8_len] = 0;
                    constant_pool.emplace_back(tag, buf);
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
        this_class = r.readu2();
        super_class = r.readu2();

        // ------------------------------ interfaces count -----------------------------

        auto interfaces_count = r.readu2();
        for (uint16_t i = 0; i < interfaces_count; i++) {
            interfaces.push_back(r.readu2());
        }

        // ----------------------------------- fields ----------------------------------

        auto fields_count = r.readu2();
        for (uint16_t i = 0; i < fields_count; i++) {
            fields.emplace_back(r);
        }

        // ----------------------------------- methods ----------------------------------

        auto methods_count = r.readu2();
        for (uint16_t i = 0; i < methods_count; i++) {
            methods.emplace_back(r);
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
        oss << "access flags: " << access_flags << endl;
        oss << "this class: " << this_class << endl;
        oss << "super class: " << super_class << endl;
        oss << "interfaces: " << interfaces.size() << endl;
        oss << "fields: " << fields.size() << endl;
        oss << "methods: " << methods.size() << endl;
        oss << "attributes: " << attributes.size() << endl;
        return oss.str();
    }
};
