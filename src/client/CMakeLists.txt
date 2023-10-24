execute_process(COMMAND /bin/sh ${PROJECT_SOURCE_DIR}/src/client/mkversion.sh
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/src/client)

execute_process(COMMAND /bin/sh ${PROJECT_SOURCE_DIR}/src/client/tools/mkautocompletion.sh
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/src/client/tools)

file(GLOB SOURCE_FILES_CLIENT_PARAMS params/*.hpp)
file(GLOB SOURCE_FILES_CLIENT_RESULTS results/*.hpp)

set(SOURCE_FILES_CLIENT
    TairResult.hpp
    TairBaseClient.cpp TairBaseClient.hpp
    TairSubscribeClient.cpp TairSubscribeClient.hpp
    TairAsyncClient.cpp TairAsyncClient.hpp
    TairClusterAsyncClient.cpp TairClusterAsyncClient.hpp
    TairClient.cpp TairClient.hpp
    TairClientWrapper.cpp TairClientWrapper.hpp
    interface/ITairClient.hpp TairClientDefine.hpp
    TairURI.cpp TairURI.hpp
    TairClientInfo.cpp TairClientInfo.hpp
    TairTransactionClient.hpp TairTransactionClient.cpp
    ${SOURCE_FILES_CLIENT_PARAMS} ${SOURCE_FILES_CLIENT_RESULTS}
    )

add_library(tair-client ${SOURCE_FILES_CLIENT})
target_link_libraries(tair-client tair-protocol)
