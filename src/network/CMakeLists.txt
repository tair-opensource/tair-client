set(SOURCE_FILES_NETWORK
    Event.hpp
    Types.hpp
    EventLoop.hpp EventLoop.cpp
    EventLoopThread.cpp EventLoopThread.hpp
    EventLoopThreadPool.cpp EventLoopThreadPool.hpp
    EventWatcher.hpp EventWatcher.cpp Timer.hpp
    Buffer.cpp Buffer.hpp
    Duration.hpp Duration.cpp
    Channel.cpp Channel.hpp
    TcpConnection.cpp TcpConnection.hpp
    DnsResolver.cpp DnsResolver.hpp
    Connector.cpp Connector.hpp
    TcpClient.cpp TcpClient.hpp
    Sockets.cpp Sockets.hpp
    Acceptor.cpp Acceptor.hpp
    TcpServer.cpp TcpServer.hpp
    NetworkStat.hpp
    TlsOptions.cpp TlsOptions.hpp
    TlsConnection.cpp TlsConnection.hpp)

add_library(tair-network ${SOURCE_FILES_NETWORK})
target_link_libraries(tair-network
    tair-common
    libevent::core libevent::extra
    ${PROJECT_SOURCE_DIR}/deps/openssl/lib/libssl.a
    ${PROJECT_SOURCE_DIR}/deps/openssl/lib/libcrypto.a)
