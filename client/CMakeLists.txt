
cmake_minimum_required(VERSION 3.0)
project(faster-im-client)

set(CMAKE_BUILD_TYPE "Debug")

# Set the path for project executable file output
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR})

# Set the search path for project compilation header files: -I
include_directories(${PROJECT_SOURCE_DIR})

aux_source_directory(.       SRC_LIST  )
aux_source_directory(./db    DB_LIST   )
aux_source_directory(./model MODEL_LIST)
aux_source_directory(./pb    PB_LIST   )
aux_source_directory(./zk    ZK_LIST   )

add_executable(imclient ${SRC_LIST} ${DB_LIST} ${MODEL_LIST} ${PB_LIST} ${ZK_LIST})
target_link_libraries(imclient pthread mysqlclient protobuf zookeeper_mt)






