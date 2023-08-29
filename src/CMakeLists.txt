
aux_source_directory(.       SRC_LIST  )
aux_source_directory(./db    DB_LIST   )
aux_source_directory(./model MODEL_LIST)
aux_source_directory(./pb    PB_LIST   )
aux_source_directory(./redis REDIS_LIST)
aux_source_directory(./zk    ZK_LIST   )

add_executable(userserver ${SRC_LIST} ${DB_LIST} ${MODEL_LIST} ${PB_LIST} ${REDIS_LIST} ${ZK_LIST})
target_link_libraries(userserver uring mysqlclient hiredis pthread protobuf zookeeper_mt)



