
# add google test dir
add_subdirectory(${PROJECT_SOURCE_DIR}/deps/googletest-release ${PROJECT_SOURCE_DIR}/deps/googletest-build EXCLUDE_FROM_ALL)
set(GTEST_LIB gtest gtest_main gmock gmock_main)

set(SOURCE_FILES_COMMON_UNIT_TEST
    common/memory/MemoryStat_test.cpp
    common/common_test.cpp
    common/ConcurrentHashMap_test.cpp
    common/Format_test.cpp
    common/Any_test.cpp
    common/FlatHash_test.cpp
    common/Mutex_test.cpp
    common/KeyHash_test.cpp
    common/MathUtil_test.cpp
    common/LatencyMetric_test.cpp
    common/Utils_test.cpp
    common/Singleton_test.cpp
    common/ThreadLocal_test.cpp
    common/ThreadLocalSingleton_test.cpp
    common/ThreadPool_test.cpp
    common/ThreadExecutorPool_test.cpp
    common/CoroutineThread_test.cpp
    common/Base64_test.cpp
    common/STL_test.cpp
    )

add_executable(common_test ${SOURCE_FILES_COMMON_UNIT_TEST})
target_link_libraries(common_test tair-common absl::flat_hash_map ${GTEST_LIB})
add_test(NAME common_test COMMAND common_test)

set(SOURCE_FILES_NETWORK_UNIT_TEST
    network/network_test.cpp
    network/Buffer_test.cpp
    network/Sockets_test.cpp
    network/EventLoopStopTest.cpp
    network/EventLoop_Timer_test.cpp
    network/EventLoopThread_Timer_test.cpp
    network/EventLoopThreadPool_Timer_test.cpp
    network/DnsResolver_test.cpp
    network/TcpServer_ConnMove_test.cpp
    network/TcpServer_Resize_IO_test.cpp
    network/TcpServer_TcpClient_test.cpp
    )

add_executable(network_test ${SOURCE_FILES_NETWORK_UNIT_TEST})
target_link_libraries(network_test tair-network ${GTEST_LIB})
add_test(NAME network_test COMMAND network_test)

set(SOURCE_FILES_PROTOCOL_UNIT_TEST
    protocol/protocol_test.cpp
    protocol/Packet_test.cpp
    protocol/memcached/MemcachedCodec_test.cpp
    protocol/resp/RESP2Codec_test.cpp
    protocol/resp/RESP3Codec_test.cpp
    protocol/resp/RESPPacketFactory_test.cpp
    protocol/resp/AttributePacket_test.cpp
    protocol/resp/BigNumberPacket_test.cpp
    protocol/resp/BlobErrorPacket_test.cpp
    protocol/resp/BooleanPacket_test.cpp
    protocol/resp/DoublePacket_test.cpp
    protocol/resp/PushPacket_test.cpp
    protocol/resp/NullPacket_test.cpp
    protocol/resp/MapPacket_test.cpp
    protocol/resp/SetPacket_test.cpp
    protocol/resp/VerbatimStringPacket_test.cpp
    protocol/resp/ArrayPacket_test.cpp
    protocol/resp/BulkStringPacket_test.cpp
    protocol/resp/ErrorPacket_test.cpp
    protocol/resp/IntegerPacket_test.cpp
    protocol/resp/SimpleStringPacket_test.cpp
    )

add_executable(protocol_test ${SOURCE_FILES_PROTOCOL_UNIT_TEST})
target_link_libraries(protocol_test tair-protocol ${GTEST_LIB})
add_test(NAME protocol_test COMMAND protocol_test)

set(SOURCE_FILES_CLIENT_UNIT_TEST
    client/client_test.cpp
    client/TairClient_Basic_test.cpp
    client/TairClient_Params_test.cpp
    client/TairClient_Standalone_Server.hpp
    client/TairClient_Standalone_Server.cpp
    client/TairClient_GenericCmd_test.cpp
    client/TairClient_StringCmd_test.cpp
    client/TairClient_ListCmd_test.cpp
    client/TairClient_SetCmd_test.cpp
    client/TairClient_HashCmd_test.cpp
    client/TairClient_ZSetCmd_test.cpp
    client/TairClient_HyperLogLogCmd_test.cpp
    client/TairClient_GeoCmd_test.cpp
    client/TairClient_StreamCmd_test.cpp
    client/TairClient_TransactionCmd_test.cpp
    client/TairClient_ScriptCmd_test.cpp)

add_executable(client_test ${SOURCE_FILES_CLIENT_UNIT_TEST})
target_link_libraries(client_test tair-client ${GTEST_LIB})
add_test(NAME client_test COMMAND client_test)