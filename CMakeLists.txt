cmake_minimum_required(VERSION 3.0)
project(mprpc)

set(CMAKE_BUILD_TYPE "Debug")
#设置项目可执行文件输出路径
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
#项目库文件输出的路径
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

#项目编译头文件搜索路径-I
include_directories(${PROJECT_SOURCE_DIR}/src/include)
include_directories(${PROJECT_SOURCE_DIR}/example)
#项目库文件搜索路径 -L
link_directories(${PROJECT_SOURCE_DIR}/lib)

#src包含mprpc框架所有代码
add_subdirectory(src)
#example包含了mprpc框架使用的示例代码
add_subdirectory(example) 
