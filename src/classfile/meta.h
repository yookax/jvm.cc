#ifndef CABIN_META_H
#define CABIN_META_H

#include <vector>
#include "../cabin.h"
#include "bytecode_reader.h"
#include "constants.h"

/*
 * 提取 Class, Field 和 Method 的公共特征
 */

struct Annotation {
    u1 *data = nullptr;
    u4 len = 0;

    void parse(BytecodeReader &r, u4 attr_len) {
        len = attr_len;
        data = new u1[len];
        memcpy(data, r.curr_pos(), len);
        r.skip(len);
    }
};

class Meta {  
public:      
    // name of Class, Field and Method
    // if is class name, 必须是全限定类名，包名之间以 '/' 分隔。
    const utf8_t *name = nullptr;
    int access_flags = 0;
    bool deprecated = false;
    const utf8_t *signature = nullptr;
    
    // std::vector<Annotation> rt_visi_annos;   // runtime visible annotations
    // std::vector<Annotation> rt_invisi_annos; // runtime invisible annotations

    Annotation rt_visi_annos;   // Runtime Visible nnotations
    Annotation rt_invisi_annos; // Runtime Invisible Annotations

    Annotation rt_visi_type_annos;  // Runtime Visible Type Annotations
    Annotation rt_invisi_type_annos;// Runtime Invisible Type Annotations

public:    
    [[nodiscard]] bool isPublic() const    { return accIsPublic(access_flags); }
    [[nodiscard]] bool isProtected() const { return accIsProtected(access_flags); }
    [[nodiscard]] bool isPrivate() const   { return accIsPrivate(access_flags); }
    [[nodiscard]] bool isStatic() const    { return accIsStatic(access_flags); }
    [[nodiscard]] bool isFinal() const     { return accIsFinal(access_flags); }
    [[nodiscard]] bool isSynthetic() const { return accIsSynthetic(access_flags); }

    void setSynthetic() { accSetSynthetic(access_flags); }
};

#endif