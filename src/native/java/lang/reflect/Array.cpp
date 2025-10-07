module;
#include "../../../../vmdef.h"

module native;

import slot;
import runtime;
import object;
import exception;

#define VERIFY_NULL    if (array == nullptr) { throw java_lang_NullPointerException(); }
#define VERIFY_ILLEGAL if (!array->is_array_object()) { throw java_lang_IllegalArgumentException("Argument is not an array"); }
#define VERIFY_BOUNDS  if (!array->check_bounds(index)) { throw java_lang_ArrayIndexOutOfBoundsException(); } // todo msg

#define GET_ARRAY_AND_INDEX \
    slot_t *args = f->lvars; \
    auto array = slot::get<jref>(args++); \
    auto index = slot::get<jint>(args++); \
    VERIFY_NULL \
    VERIFY_ILLEGAL \
    VERIFY_BOUNDS

#define VERIFY_BOOL_ARRAY \
if (!array->is_bool_array()) { \
    throw java_lang_IllegalArgumentException("Array is not a bool array."); \
}

#define VERIFY_BYTE_ARRAY \
if (!array->is_byte_array()) { \
    throw java_lang_IllegalArgumentException("Array is not a byte array."); \
}

#define VERIFY_CHAR_ARRAY \
if (!array->is_char_array()) { \
    throw java_lang_IllegalArgumentException("Array is not a char array."); \
}

#define VERIFY_SHORT_ARRAY \
if (!array->is_short_array()) { \
    throw java_lang_IllegalArgumentException("Array is not a short array."); \
}

#define VERIFY_INT_ARRAY \
if (!array->is_int_array()) { \
    throw java_lang_IllegalArgumentException("Array is not a int array."); \
}

#define VERIFY_LONG_ARRAY \
if (!array->is_long_array()) { \
    throw java_lang_IllegalArgumentException("Array is not a long array."); \
}

#define VERIFY_FLOAT_ARRAY \
if (!array->is_float_array()) { \
    throw java_lang_IllegalArgumentException("Array is not a float array."); \
}

#define VERIFY_DOUBLE_ARRAY \
if (!array->is_double_array()) { \
    throw java_lang_IllegalArgumentException("Array is not a double array."); \
}

//public static native int getLength(Object array) throws IllegalArgumentException;
void getLength(Frame *f) {
    slot_t *args = f->lvars;
    auto array = slot::get<jref>(args);
    VERIFY_NULL
    VERIFY_ILLEGAL
    f->pushi(array->array_len());
}


//public static native Object get(Object array, int index)
//    throws IllegalArgumentException, ArrayIndexOutOfBoundsException;
void get(Frame *f) {
    GET_ARRAY_AND_INDEX
    jref o;
    switch (array->clazz->name[1]) {
        case 'Z': // boolean[]
            o = bool_box(array->getElt<jbool>(index));
            break;
        case 'B': // byte[]
            o = byte_box(array->getElt<jbyte>(index));
            break;
        case 'C': // char[]
            o = char_box(array->getElt<jchar>(index));
            break;
        case 'S': // short[]
            o = short_box(array->getElt<jshort>(index));
            break;
        case 'I': // int[]
            o = int_box(array->getElt<jint>(index));
            break;
        case 'J': // long[]
            o = long_box(array->getElt<jlong>(index));
            break;
        case 'F': // float[]
            o = float_box(array->getElt<jfloat>(index));
            break;
        case 'D': // double[]
            o = double_box(array->getElt<jdouble>(index));
            break;
        default:  // reference array
            o = array->getElt<jref>(index);
            break;
    }
    f->pushr(o);
}

    /**
     * Returns the value of the indexed component in the specified
     * array object, as a {@code boolean}.
     *
     * @param array the array
     * @param index the index
     * @return the value of the indexed component in the specified array
     * @throws    NullPointerException If the specified object is null
     * @throws    IllegalArgumentException If the specified object is not
     * an array, or if the indexed element cannot be converted to the
     * return type by an identity or widening conversion
     * @throws    ArrayIndexOutOfBoundsException If the specified {@code index}
     * argument is negative, or if it is greater than or equal to the
     * length of the specified array
     * @see Array#get
     */
//public static native boolean getBoolean(Object array, int index)
//    throws IllegalArgumentException, ArrayIndexOutOfBoundsException;
void getBoolean(Frame *f) {
    GET_ARRAY_AND_INDEX
    VERIFY_BOOL_ARRAY
    auto x = array->getElt<jbool>(index);
    f->pushi(x);
}

//public static native byte getByte(Object array, int index)
//    throws IllegalArgumentException, ArrayIndexOutOfBoundsException;
void getByte(Frame *f) {
    GET_ARRAY_AND_INDEX
    VERIFY_BYTE_ARRAY
    auto x = array->getElt<jbyte>(index);
    f->pushi(x);
}

//public static native char getChar(Object array, int index)
//    throws IllegalArgumentException, ArrayIndexOutOfBoundsException;
void getChar(Frame *f) {
    GET_ARRAY_AND_INDEX
    VERIFY_CHAR_ARRAY
    auto x = array->getElt<jchar>(index);
    f->pushi(x);
}

//public static native short getShort(Object array, int index)
//    throws IllegalArgumentException, ArrayIndexOutOfBoundsException;
void getShort(Frame *f) {
    GET_ARRAY_AND_INDEX
    VERIFY_SHORT_ARRAY
    auto x = array->getElt<jshort>(index);
    f->pushi(x);
}

//public static native int getInt(Object array, int index)
//    throws IllegalArgumentException, ArrayIndexOutOfBoundsException;
void getInt(Frame *f) {
    GET_ARRAY_AND_INDEX
    VERIFY_INT_ARRAY
    auto x = array->getElt<jint>(index);
    f->pushi(x);
}

//public static native long getLong(Object array, int index)
//    throws IllegalArgumentException, ArrayIndexOutOfBoundsException;
void getLong(Frame *f) {
    GET_ARRAY_AND_INDEX
    VERIFY_LONG_ARRAY
    auto x = array->getElt<jlong>(index);
    f->pushl(x);
}

//public static native float getFloat(Object array, int index)
//    throws IllegalArgumentException, ArrayIndexOutOfBoundsException;
void getFloat(Frame *f) {
    GET_ARRAY_AND_INDEX
    VERIFY_FLOAT_ARRAY
    auto x = array->getElt<jfloat>(index);
    f->pushf(x);
}

//public static native double getDouble(Object array, int index)
//    throws IllegalArgumentException, ArrayIndexOutOfBoundsException;
void getDouble(Frame *f) {
    GET_ARRAY_AND_INDEX
    VERIFY_DOUBLE_ARRAY
    auto x = array->getElt<jdouble>(index);
    f->pushd(x);
}

//public static native void set(Object array, int index, Object value)
//    throws IllegalArgumentException, ArrayIndexOutOfBoundsException;
void set(Frame *f) {
    GET_ARRAY_AND_INDEX
    jref value = slot::get<jref>(args);
    if (value == nullptr) {
        if (array->is_type_array()) {
            // 基本类型的数组无法设空值
            throw java_lang_IllegalArgumentException(); // todo msg
        }
        array->setRefElt(index, value);
        return;
    }

    switch (array->clazz->name[1]) {
        case 'Z': // boolean[]
            if (strcmp(value->clazz->name, "java/lang/Boolean") != 0) {
                throw java_lang_IllegalArgumentException("argument type mismatch");
            } else {
                array->setBoolElt(index, slot::get<jbool>(value->unbox()));
            }
            return;
        case 'B': // byte[]
            if (strcmp(value->clazz->name, "java/lang/Byte") != 0) {
                throw java_lang_IllegalArgumentException("argument type mismatch");
            } else {
                array->setByteElt(index, slot::get<jbyte>(value->unbox()));
            }
            return;
        case 'C': // char[]
            if (strcmp(value->clazz->name, "java/lang/Character") != 0) {
                throw java_lang_IllegalArgumentException("argument type mismatch");
            } else {
                array->setCharElt(index, slot::get<jchar>(value->unbox()));
            }
            return;
        case 'S': // short[]
            if (strcmp(value->clazz->name, "java/lang/Short") != 0) {
                throw java_lang_IllegalArgumentException("argument type mismatch");
            } else {
                array->setShortElt(index, slot::get<jshort>(value->unbox()));
            }
            return;
        case 'I': // int[]
            if (strcmp(value->clazz->name, "java/lang/Integer") != 0) {
                throw java_lang_IllegalArgumentException("argument type mismatch");
            } else {
                array->setIntElt(index, slot::get<jint>(value->unbox()));
            }
            return;
        case 'J': // long[]
            if (strcmp(value->clazz->name, "java/lang/Long") != 0) {
                throw java_lang_IllegalArgumentException("argument type mismatch");
            } else {
                array->setLongElt(index, slot::get<jlong>(value->unbox()));
            }
            return;
        case 'F': // float[]
            if (strcmp(value->clazz->name, "java/lang/Float") != 0) {
                throw java_lang_IllegalArgumentException("argument type mismatch");
            } else {
                array->setFloatElt(index, slot::get<jfloat>(value->unbox()));
            }
            return;
        case 'D': // double[]
            if (strcmp(value->clazz->name, "java/lang/Double") != 0) {
                throw java_lang_IllegalArgumentException("argument type mismatch");
            } else {
                array->setDoubleElt(index, slot::get<jdouble>(value->unbox()));
            }
            return;
        default:  // reference array
            array->setRefElt(index, value);
            return;
    }
}

//public static native void setBoolean(Object array, int index, boolean z)
//    throws IllegalArgumentException, ArrayIndexOutOfBoundsException;
void setBoolean(Frame *f) {
    GET_ARRAY_AND_INDEX
    VERIFY_BOOL_ARRAY
    auto x = slot::get<jbool>(args);
    array->setBoolElt(index, x);
}

//public static native void setByte(Object array, int index, byte b)
//    throws IllegalArgumentException, ArrayIndexOutOfBoundsException;
void setByte(Frame *f) {
    GET_ARRAY_AND_INDEX
    VERIFY_BYTE_ARRAY
    auto x = slot::get<jbyte>(args);
    array->setByteElt(index, x);
}

//public static native void setChar(Object array, int index, char c)
//    throws IllegalArgumentException, ArrayIndexOutOfBoundsException;
void setChar(Frame *f) {
    GET_ARRAY_AND_INDEX
    VERIFY_CHAR_ARRAY
    auto x = slot::get<jchar>(args);
    array->setCharElt(index, x);
}

//public static native void setShort(Object array, int index, short s)
//    throws IllegalArgumentException, ArrayIndexOutOfBoundsException;
void setShort(Frame *f) {
    GET_ARRAY_AND_INDEX
    VERIFY_SHORT_ARRAY
    auto x = slot::get<jshort>(args);
    array->setShortElt(index, x);
}

//public static native void setInt(Object array, int index, int i)
//    throws IllegalArgumentException, ArrayIndexOutOfBoundsException;
void setInt(Frame *f) {
    GET_ARRAY_AND_INDEX
    VERIFY_INT_ARRAY
    auto x = slot::get<jint>(args);
    array->setIntElt(index, x);
}

// public static native void setLong(Object array, int index, long l)
//    throws IllegalArgumentException, ArrayIndexOutOfBoundsException;
void setLong(Frame *f) {
    GET_ARRAY_AND_INDEX
    VERIFY_LONG_ARRAY
    auto x = slot::get<jlong>(args);
    array->setLongElt(index, x);
}

// public static native void setFloat(Object array, int index, float f)
//    throws IllegalArgumentException, ArrayIndexOutOfBoundsException;
void setFloat(Frame *f) {
    GET_ARRAY_AND_INDEX
    VERIFY_FLOAT_ARRAY
    auto x = slot::get<jfloat>(args);
    array->setFloatElt(index, x);
}

// public static native void setDouble(Object array, int index, double d)
//    throws IllegalArgumentException, ArrayIndexOutOfBoundsException;
void setDouble(Frame *f) {
    GET_ARRAY_AND_INDEX
    VERIFY_DOUBLE_ARRAY
    auto x = slot::get<jdouble>(args);
    array->setDoubleElt(index, x);
}

// private static native Object newArray(Class<?> componentType, int length)
//    throws NegativeArraySizeException;
void newArray(Frame *f) {
    slot_t *args = f->lvars;
    auto component_type = slot::get<jref>(args++);
    auto length = slot::get<jint>(args);

    if (component_type == nullptr) {
        throw java_lang_NullPointerException();
    }
    if (length < 0) {
        throw java_lang_NegativeArraySizeException();  // todo msg
    }

    jref o = Allocator::array(component_type->jvm_mirror->generate_array_class(), length);
    f->pushr(o);
}

// private static native Object multiNewArray(Class<?> componentType, int[] dimensions)
//    throws IllegalArgumentException, NegativeArraySizeException;
void multiNewArray(Frame *f) {
    slot_t *args = f->lvars;
    auto component_type = slot::get<jref>(args++);
    auto dimensions = slot::get<jref>(args);

    if (component_type == nullptr) {
        throw java_lang_NullPointerException();
    }

    Class *c = component_type->jvm_mirror;
    jsize dim_count = dimensions->array_len();
    auto lens = new jint[dim_count];
    for (jsize i = 0; i < dim_count; i++) {
        lens[i] = dimensions->getElt<jint>(i);
        c = c->generate_array_class();
    }

    ArrayClass *ac = (ArrayClass *) c;
    jref a = Allocator::multi_array(ac, dim_count, lens);
    delete[] lens;
    f->pushr(a);
}

void java_lang_reflect_Array_registerNatives() {
#undef R
#define R(method, method_descriptor) \
    registry("java/lang/reflect/Array", #method, method_descriptor, method)

    R(getLength, "(Ljava/lang/Object;)I");

    R(get, "(Ljava/lang/Object;I)Ljava/lang/Object;");
    R(getBoolean, "(Ljava/lang/Object;I)Z");
    R(getByte, "(Ljava/lang/Object;I)B");
    R(getChar, "(Ljava/lang/Object;I)C");
    R(getShort, "(Ljava/lang/Object;I)S");
    R(getInt, "(Ljava/lang/Object;I)I");
    R(getLong, "(Ljava/lang/Object;I)J");
    R(getFloat, "(Ljava/lang/Object;I)F");
    R(getDouble, "(Ljava/lang/Object;I)D");

    R(set, "(Ljava/lang/Object;ILjava/lang/Object;)V");
    R(setBoolean, "(Ljava/lang/Object;IZ)V");
    R(setByte, "(Ljava/lang/Object;IB)V");
    R(setChar, "(Ljava/lang/Object;IC)V");
    R(setShort, "(Ljava/lang/Object;IS)V");
    R(setInt, "(Ljava/lang/Object;II)V");
    R(setLong, "(Ljava/lang/Object;IJ)V");
    R(setFloat, "(Ljava/lang/Object;IF)V");
    R(setDouble, "(Ljava/lang/Object;ID)V");

    R(newArray, "(Ljava/lang/Class;I)Ljava/lang/Object;");
    R(multiNewArray, "(Ljava/lang/Class;[I)Ljava/lang/Object;");
}