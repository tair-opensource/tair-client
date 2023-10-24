/*
 *  Copyright (c) 2023 Tair
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in all
 *  copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *  SOFTWARE.
 */
#pragma once

#include <algorithm>

#include "common/Assert.hpp"
#include "common/Copyable.hpp"
#include "common/Endianconv.hpp"
#include "common/StringUtil.hpp"

#include "fmt/format.h"

namespace tair::network {

using common::Copyable;
using common::StringUtil;

using socket_t = int;

//
// Modified from muduo project http://github.com/chenshuo/muduo
//
// A buffer class modeled after org.jboss.netty.buffer.ChannelBuffer
//
// +-------------------+------------------+------------------+
// | prependable bytes |  readable bytes  |  writable bytes  |
// |                   |     (CONTENT)    |                  |
// +-------------------+------------------+------------------+
// |                   |                  |                  |
// 0      <=      readerIndex   <=   writerIndex    <=     size
//
class Buffer : public Copyable {
public:
    static const size_t kCheapPrependSize;
    static const size_t kInitialSize;

    // just for stat
    enum BufferType {
        GENERAL_BUFFER,
        INPUT_BUFFER,
        OUTPUT_BUFFER,
    };

    explicit Buffer(BufferType type = GENERAL_BUFFER,
                    size_t initial_size = kInitialSize,
                    size_t reserved_prepend_size = kCheapPrependSize)
        : type_(type),
          capacity_(reserved_prepend_size + initial_size),
          read_index_(reserved_prepend_size),
          write_index_(reserved_prepend_size),
          reserved_prepend_size_(reserved_prepend_size) {
        buffer_ = new char[capacity_];
        runtimeAssert(length() == 0);
        runtimeAssert(writableBytes() == initial_size);
        runtimeAssert(prependableBytes() == reserved_prepend_size);
        addBufferStatSize(buffer_);
    }

    ~Buffer() {
        subBufferStatSize(buffer_);
        delete[] buffer_;
        buffer_ = nullptr;
        capacity_ = 0;
        read_index_ = 0;
        write_index_ = 0;
        reserved_prepend_size_ = 0;
    }

    Buffer(Buffer &&rhs) noexcept {
        std::swap(type_, rhs.type_);
        std::swap(buffer_, rhs.buffer_);
        std::swap(capacity_, rhs.capacity_);
        std::swap(read_index_, rhs.read_index_);
        std::swap(write_index_, rhs.write_index_);
        std::swap(reserved_prepend_size_, rhs.reserved_prepend_size_);
    }

    Buffer &operator=(Buffer &&rhs) noexcept {
        std::swap(type_, rhs.type_);
        std::swap(buffer_, rhs.buffer_);
        std::swap(capacity_, rhs.capacity_);
        std::swap(read_index_, rhs.read_index_);
        std::swap(write_index_, rhs.write_index_);
        std::swap(reserved_prepend_size_, rhs.reserved_prepend_size_);
        return *this;
    }

    Buffer(const Buffer &rhs) {
        type_ = rhs.type_;
        capacity_ = rhs.capacity_;
        read_index_ = rhs.read_index_;
        write_index_ = rhs.write_index_;
        reserved_prepend_size_ = rhs.reserved_prepend_size_;
        buffer_ = new char[capacity_];
        runtimeAssert(write_index_ >= read_index_);
        memcpy(buffer_ + read_index_, rhs.buffer_ + read_index_, write_index_ - read_index_);
        addBufferStatSize(buffer_);
    }

    Buffer &operator=(const Buffer &rhs) {
        if (&rhs == this) {
            return *this;
        }
        subBufferStatSize(buffer_);
        delete[] buffer_;
        type_ = rhs.type_;
        capacity_ = rhs.capacity_;
        read_index_ = rhs.read_index_;
        write_index_ = rhs.write_index_;
        reserved_prepend_size_ = rhs.reserved_prepend_size_;
        buffer_ = new char[capacity_];
        runtimeAssert(write_index_ >= read_index_);
        memcpy(buffer_ + read_index_, rhs.buffer_ + read_index_, write_index_ - read_index_);
        addBufferStatSize(buffer_);
        return *this;
    }

    void swap(Buffer &rhs) {
        std::swap(type_, rhs.type_);
        std::swap(buffer_, rhs.buffer_);
        std::swap(capacity_, rhs.capacity_);
        std::swap(read_index_, rhs.read_index_);
        std::swap(write_index_, rhs.write_index_);
        std::swap(reserved_prepend_size_, rhs.reserved_prepend_size_);
    }

    BufferType getType() const {
        return type_;
    }

    // Reinit to empty buffer
    void reinit() {
        subBufferStatSize(buffer_);
        delete[] buffer_;
        capacity_ = kInitialSize + kCheapPrependSize;
        read_index_ = kCheapPrependSize;
        write_index_ = kCheapPrependSize;
        reserved_prepend_size_ = kCheapPrependSize;
        buffer_ = new char[capacity_];
        addBufferStatSize(buffer_);
    }

    // Skip advances the reading index of the buffer
    void skip(size_t len) {
        if (len < length()) {
            read_index_ += len;
        } else {
            reset();
        }
    }

    // Retrieve advances the reading index of the buffer
    // Retrieve it the same as Skip.
    void retrieve(size_t len) {
        skip(len);
    }

    // Truncate discards all but the first n unread bytes from the buffer
    // but continues to use the same allocated storage.
    // It does nothing if n is greater than the length of the buffer.
    void truncate(size_t n) {
        if (n == 0) {
            read_index_ = reserved_prepend_size_;
            write_index_ = reserved_prepend_size_;
        } else if (write_index_ > read_index_ + n) {
            write_index_ = read_index_ + n;
        }
    }

    // Reset resets the buffer to be empty,
    // but it retains the underlying storage for use by future writes.
    // Reset is the same as Truncate(0).
    void reset() {
        truncate(0);
    }

    void clear() {
        truncate(0);
    }

    // Increase the capacity of the container to a value that's greater
    // or equal to len. If len is greater than the current capacity(),
    // new storage is allocated, otherwise the method does nothing.
    void reserve(size_t len) {
        if (capacity_ >= len + reserved_prepend_size_) {
            return;
        }
        grow(len + reserved_prepend_size_);
    }

    // Make sure there is enough memory space to append more data with length len
    void ensureWritableBytes(size_t len) {
        if (writableBytes() < len) {
            grow(len);
        }
        runtimeAssert(writableBytes() >= len);
    }

    // toText appends char '\0' to buffer to convert the underlying data to a c-style string text.
    // It will not change the length of buffer.
    void toText() {
        appendInt8('\0');
        unwriteBytes(1);
    }

public:
    void write(const void *d, size_t len) {
        ensureWritableBytes(len);
        memcpy(writeBegin(), d, len);
        runtimeAssert(write_index_ + len <= capacity_);
        write_index_ += len;
    }

    void incrWriteIndex(size_t len) {
        write_index_ += len;
    }

    void write(const char *str) {
        write(str, strlen(str));
    }

    void write(const std::string &str) {
        write(str.data(), str.size());
    }

    void append(const char *str) {
        write(str, strlen(str));
    }

    void append(const std::string &str) {
        write(str.data(), str.size());
    }

    void append(const std::string_view &str) {
        write(str.data(), str.size());
    }

    void append(const char *d, size_t len) {
        write(d, len);
    }

    void append(const void *d, size_t len) {
        write(d, len);
    }

    void appendCRLF() {
        write(kCRLF, 2);
    }

    // Append int64_t/int32_t/int16_t with network endian
    void appendInt64(int64_t x) {
        int64_t be = htonu64(x);
        write(&be, sizeof(be));
    }

    void appendInt32(int32_t x) {
        int32_t be32 = htonu32(x);
        write(&be32, sizeof(be32));
    }

    void appendInt16(int16_t x) {
        int16_t be16 = htonu16(x);
        write(&be16, sizeof(be16));
    }

    void appendInt8(int8_t x) {
        write(&x, sizeof(x));
    }

    template <typename T>
    void appendNumberToStr(T number) {
        ensureWritableBytes(32);
        char *end = fmt::format_to(writeBegin(), "{}", number);
        size_t len = end - writeBegin();
        runtimeAssert(write_index_ + len <= capacity_);
        write_index_ += len;
    }

    // Prepend int64_t/int32_t/int16_t with network endian
    void prependInt64(int64_t x) {
        int64_t be = htonu64(x);
        prepend(&be, sizeof(be));
    }

    void prependInt32(int32_t x) {
        int32_t be32 = htonu32(x);
        prepend(&be32, sizeof(be32));
    }

    void prependInt16(int16_t x) {
        int16_t be16 = htonu16(x);
        prepend(&be16, sizeof(be16));
    }

    void prependInt8(int8_t x) {
        prepend(&x, sizeof(x));
    }

    // Insert content, specified by the parameter, into the front of reading index
    void prepend(const void *d, size_t len) {
        runtimeAssert(len <= prependableBytes());
        read_index_ -= len;
        const char *p = static_cast<const char *>(d);
        memcpy(begin() + read_index_, p, len);
    }

    void prepend(const char *str) {
        prepend(str, strlen(str));
    }

    void unwriteBytes(size_t n) {
        runtimeAssert(n <= length());
        write_index_ -= n;
    }

    void writeBytes(size_t n) {
        runtimeAssert(n <= writableBytes());
        write_index_ += n;
    }

public:
    int64_t readInt64() {
        int64_t result = peekInt64();
        skip(sizeof(result));
        return result;
    }

    int32_t readInt32() {
        int32_t result = peekInt32();
        skip(sizeof(result));
        return result;
    }

    int16_t readInt16() {
        int16_t result = peekInt16();
        skip(sizeof(result));
        return result;
    }

    int8_t readInt8() {
        int8_t result = peekInt8();
        skip(sizeof(result));
        return result;
    }

    std::string_view toStringView() const {
        return std::string_view(data(), length());
    }

    std::string toString() const {
        return std::string(data(), length());
    }

    std::string toDebugString(size_t max_size) const {
        if (max_size > write_index_ - reserved_prepend_size_) {
            max_size = write_index_ - reserved_prepend_size_;
        }
        return fmt::format("ridx: {}, widx: {}, data: {}",
                           read_index_ - reserved_prepend_size_,
                           write_index_ - reserved_prepend_size_,
                           StringUtil::toPrintableStr(std::string_view(buffer_ + reserved_prepend_size_, max_size)));
    }

    void shrink(size_t reserve) {
        Buffer other(type_, length() + reserve, reserved_prepend_size_);
        other.append(toStringView());
        swap(other);
    }

    // readFromFd reads data from a fd directly into buffer,
    // and return result of readv, errno is saved into saved_errno
    ssize_t readFromFd(socket_t fd, int *saved_errno);

    // Next returns a std::string_view containing the next n bytes from the buffer,
    // advancing the buffer as if the bytes had been returned by Read.
    // If there are fewer than n bytes in the buffer, Next returns the entire buffer.
    // The std::string_view is only valid until the next call to a read or write method.
    std::string_view next(size_t len) {
        if (len < length()) {
            std::string_view result(data(), len);
            read_index_ += len;
            return result;
        }
        return nextAll();
    }

    // NextAll returns a std::string_view containing all the unread portion of the buffer,
    // advancing the buffer as if the bytes had been returned by Read.
    std::string_view nextAll() {
        std::string_view result(data(), length());
        reset();
        return result;
    }

    std::string nextString(size_t len) {
        std::string_view s = next(len);
        return std::string(s);
    }

    std::string nextAllString() {
        std::string_view s = nextAll();
        return std::string(s);
    }

    // ReadByte reads and returns the next byte from the buffer.
    // If no byte is available, it returns '\0'.
    char readByte() {
        runtimeAssert(length() >= 1);
        if (length() == 0) {
            return '\0';
        }
        return buffer_[read_index_++];
    }

    // UnreadBytes unreads the last n bytes returned
    // by the most recent read operation.
    void unreadBytes(size_t n) {
        runtimeAssert(n < read_index_);
        read_index_ -= n;
    }

public:
    int64_t peekInt64() const {
        runtimeAssert(length() >= sizeof(int64_t));
        int64_t be64 = 0;
        ::memcpy(&be64, data(), sizeof be64);
        return ntohu64(be64);
    }

    int32_t peekInt32() const {
        runtimeAssert(length() >= sizeof(int32_t));
        int32_t be32 = 0;
        ::memcpy(&be32, data(), sizeof be32);
        return ntohu32(be32);
    }

    int16_t peekInt16() const {
        runtimeAssert(length() >= sizeof(int16_t));
        int16_t be16 = 0;
        ::memcpy(&be16, data(), sizeof be16);
        return ntohu16(be16);
    }

    int8_t peekInt8() const {
        runtimeAssert(length() >= sizeof(int8_t));
        int8_t x = *data();
        return x;
    }

public:
    // data returns a pointer of length Buffer.length() holding the unread portion of the buffer.
    // The data is valid for use only until the next buffer modification (that is,
    // only until the next call to a method like Read, Write, Reset, or Truncate).
    // The data aliases the buffer content at least until the next buffer modification,
    // so immediate changes to the std::string_view will affect the result of future reads.
    const char *data() const {
        return buffer_ + read_index_;
    }

    char *writeBegin() {
        return begin() + write_index_;
    }

    const char *writeBegin() const {
        return begin() + write_index_;
    }

    // length returns the number of bytes of the unread portion of the buffer
    size_t length() const {
        runtimeAssert(write_index_ >= read_index_);
        return write_index_ - read_index_;
    }

    bool empty() const {
        return (length() == 0);
    }

    // size returns the number of bytes of the unread portion of the buffer.
    // It is the same as length().
    size_t size() const {
        return length();
    }

    // capacity returns the capacity of the buffer's underlying byte std::string_view, that is, the
    // total space allocated for the buffer's data.
    size_t capacity() const {
        return capacity_;
    }

    size_t writableBytes() const {
        runtimeAssert(capacity_ >= write_index_);
        return capacity_ - write_index_;
    }

    size_t prependableBytes() const {
        return read_index_;
    }

public:
    const char *findCRLF() const {
        const char *crlf = std::search(data(), writeBegin(), kCRLF, kCRLF + 2);
        return crlf == writeBegin() ? nullptr : crlf;
    }

    const char *findCRLF(const char *start) const {
        runtimeAssert(data() <= start);
        runtimeAssert(start <= writeBegin());
        const char *crlf = std::search(start, writeBegin(), kCRLF, kCRLF + 2);
        return crlf == writeBegin() ? nullptr : crlf;
    }

    const char *findEOL() const {
        const void *eol = memchr(data(), '\n', length());
        return static_cast<const char *>(eol);
    }

    const char *findEOL(const char *start) const {
        runtimeAssert(data() <= start);
        runtimeAssert(start <= writeBegin());
        const void *eol = memchr(start, '\n', writeBegin() - start);
        return static_cast<const char *>(eol);
    }

private:
    char *begin() {
        return buffer_;
    }

    const char *begin() const {
        return buffer_;
    }

    void grow(size_t len) {
        if (writableBytes() + prependableBytes() < len + reserved_prepend_size_) {
            // grow the capacity
            size_t n = (capacity_ << 1) + len;
            size_t m = length();
            char *new_buf = new char[n];
            memcpy(new_buf + reserved_prepend_size_, begin() + read_index_, m);
            write_index_ = m + reserved_prepend_size_;
            read_index_ = reserved_prepend_size_;
            capacity_ = n;
            subBufferStatSize(buffer_);
            delete[] buffer_;
            buffer_ = new_buf;
            addBufferStatSize(buffer_);
        } else {
            // move readable data to the front, make space inside buffer
            runtimeAssert(reserved_prepend_size_ < read_index_);
            size_t readable = length();
            memmove(begin() + reserved_prepend_size_, begin() + read_index_, length());
            read_index_ = reserved_prepend_size_;
            write_index_ = read_index_ + readable;
            runtimeAssert(readable == length());
            runtimeAssert(writableBytes() >= len);
        }
    }

    void addBufferStatSize(void *ptr);
    void subBufferStatSize(void *ptr);

private:
    BufferType type_ = GENERAL_BUFFER;
    char *buffer_ = nullptr;
    size_t capacity_ = 0;
    size_t read_index_ = 0;
    size_t write_index_ = 0;
    size_t reserved_prepend_size_ = 0;
    static const char kCRLF[];
};

} // namespace tair::network
