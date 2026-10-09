# 指定要求cmake的最低版本
cmake_minimum_required(VERSION 3.0)
project(Auth)

#设置C++ 编译的参数(根据需要自己设置) PS：可以参考esclipse中编译配置的选项
set(CMAKE_CXX_FLAGS "-std=c++11 -pthread -Wall -O0 -s -fmessage-length=0 -fPIC")

set(CMAKE_BUILD_TYPE "realse")  

# 程序所引用的头文件库目录。
include_directories(
    ${PROJECT_SOURCE_DIR}
)

# 查找要需要编译的源文件。
# file函数将递归查文件夹及子文件夹下的所有.cpp 和 .c文件放到SRC中
# list函数排除build下面的.cpp和.c文件(在make过程中,会在bulid下面产生测试的cpp和c文件，避免冲突)
# 这样我们将所需要编译的源文件放到SRC中
file(GLOB_RECURSE SRC *.cpp *.c)

# 不编译NanJingFord项目
file(GLOB_RECURSE SRC_EXPECT build/*.cpp build/*.c)
list(REMOVE_ITEM SRC ${SRC_EXPECT})

# 设置生成库的名字
set(SRC_NAME "license_generator")
# 编译生成共享库
add_executable("${SRC_NAME}"  ${SRC})
# ${LIB_NAME} : 可执行文件
# ${SRC} ：需要编译的源文件


# 连接动态库
# target_link_libraries(${SRC_NAME})
                        