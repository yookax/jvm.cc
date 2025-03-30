

# 查找子目录中的所有源文件
file(GLOB MINIZIP_FILES *.c)

# 将这些文件传递给父目录
set(MINIZIP_FILES ${MINIZIP_FILES} PARENT_SCOPE)