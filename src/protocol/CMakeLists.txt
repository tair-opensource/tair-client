set(SOURCE_FILES_PROTOCOL
    packet/Packet.hpp
    packet/resp/RESPPacketFactory.cpp packet/resp/RESPPacketFactory.hpp
    packet/resp/SimpleStringPacket.hpp
    packet/resp/ErrorPacket.hpp
    packet/resp/IntegerPacket.hpp
    packet/resp/BulkStringPacket.cpp packet/resp/BulkStringPacket.hpp
    packet/resp/ArrayPacket.hpp packet/resp/ArrayPacket.cpp
    packet/resp/NullPacket.hpp
    packet/resp/DoublePacket.hpp
    packet/resp/BooleanPacket.hpp
    packet/resp/BlobErrorPacket.hpp
    packet/resp/VerbatimStringPacket.hpp
    packet/resp/MapPacket.cpp packet/resp/MapPacket.hpp
    packet/resp/AttributePacket.hpp
    packet/resp/SetPacket.hpp
    packet/resp/PushPacket.hpp
    packet/resp/BigNumberPacket.hpp
    packet/resp/RESPPacketHelper.hpp
    packet/memcached/MemcachedIncDecPacket.hpp
    packet/memcached/MemcachedStatusPacket.hpp
    packet/memcached/MemcachedStrPacket.hpp
    packet/memcached/MemcachedTouchPacket.hpp
    packet/memcached/MemcachedGetPacket.hpp
    packet/memcached/MemcachedMscanPacket.hpp
    codec/Codec.hpp
    codec/CodecAble.hpp
    codec/CodecFactory.hpp
    codec/CodecType.hpp
    codec/DState.hpp
    codec/resp/RESPProtocol.hpp
    codec/resp/RESP2Codec.cpp codec/resp/RESP2Codec.hpp
    codec/resp/RESP2CodecAble.hpp
    codec/resp/RESP3Codec.cpp codec/resp/RESP3Codec.hpp
    codec/resp/RESP3CodecAble.hpp
    codec/memcached/MemcachedProtocol.hpp
    codec/memcached/MemcachedCodec.cpp codec/memcached/MemcachedCodec.hpp
    codec/memcached/MemcachedCodecAble.hpp
    ProtocolOptions.cpp ProtocolOptions.hpp)

add_library(tair-protocol ${SOURCE_FILES_PROTOCOL})
target_link_libraries(tair-protocol tair-network)
