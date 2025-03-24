aux_source_directory(. SRC_LIST)
# shared 生成动态库
# add_library(mprpc SHARED ${SRC_LIST})
add_library(mprpc ${SRC_LIST})
#muduo是静态库
target_link_libraries(mprpc muduo_net muduo_base pthread zookeeper_mt)

