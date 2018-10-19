# -*- coding: utf-8 -*-
import os


def get_all_classes(root_dir):
    """
    返回给定目录下的所有 class 文件
    """
    classes = []

    for lists in os.listdir(root_dir):
        path = os.path.join(root_dir, lists)
        if not os.path.isdir(path) and path.endswith(".java"):  # 每个 .java 文件对应一个可执行的 .class文件（有main函数）
            path = path.replace(".java", "")  # 删除 .java 后缀
            classes.append(path)

    return classes


if __name__ == '__main__':
    # 生成的 classes 文件的存放目录
    # jvm_path = "D:/code/jvm/cmake-build-release/jvm.exe -printlevel 3 -bootstrapclasspath 'C:\Program Files\Java\jre1.8.0_162\lib' "
    jvm_path = "D:/code/jvm/cmake-build-debug/jvm.exe -printlevel 3 -bootstrapclasspath 'C:\Program Files\Java\jre1.8.0_162\lib' "
    all_classes = get_all_classes("test/")
    for clazz in all_classes:
        cmd = jvm_path + clazz
        print(clazz)
        # os.system(cmd)
        