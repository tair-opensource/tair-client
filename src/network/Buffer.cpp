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
#include "network/Buffer.hpp"

#include <sys/uio.h>

#include "common/MemoryStat.hpp"
#include "network/NetworkStat.hpp"
#include "network/Sockets.hpp"

namespace tair::network {

const char Buffer::kCRLF[] = "\r\n";

const size_t Buffer::kCheapPrependSize = 8;
const size_t Buffer::kInitialSize = 1024 * 16 - kCheapPrependSize;

void Buffer::addBufferStatSize(void *ptr) {
    if (type_ == INPUT_BUFFER) {
        NetworkStat::addNetInputBufferSize(common::tair_malloc_usable_size(ptr));
    } else if (type_ == OUTPUT_BUFFER) {
        NetworkStat::addNetOutputBufferSize(common::tair_malloc_usable_size(ptr));
    } else {
        NetworkStat::addGeneralBufferSize(common::tair_malloc_usable_size(ptr));
    }
}

void Buffer::subBufferStatSize(void *ptr) {
    if (type_ == INPUT_BUFFER) {
        NetworkStat::subNetInputBufferSize(common::tair_malloc_usable_size(ptr));
    } else if (type_ == OUTPUT_BUFFER) {
        NetworkStat::subNetOutputBufferSize(common::tair_malloc_usable_size(ptr));
    } else {
        NetworkStat::subGeneralBufferSize(common::tair_malloc_usable_size(ptr));
    }
}

ssize_t Buffer::readFromFd(socket_t fd, int *saved_errno) {
    // saved an ioctl()/FIONREAD call to tell how much to read
    char extrabuf[65536];
    struct iovec vec[2];
    const size_t writable = writableBytes();
    vec[0].iov_base = begin() + write_index_;
    vec[0].iov_len = writable;
    vec[1].iov_base = extrabuf;
    vec[1].iov_len = sizeof(extrabuf);
    // when there is enough space in this buffer, don't read into extrabuf.
    // when extrabuf is used, we read 64k bytes at most.
    const int iovcnt = (writable < sizeof(extrabuf)) ? 2 : 1;
    const ssize_t n = sockets::readVFromSocket(fd, vec, iovcnt);

    if (n < 0) {
        *saved_errno = errno;
    } else if (static_cast<size_t>(n) <= writable) {
        write_index_ += n;
    } else {
        write_index_ = capacity_;
        append(extrabuf, n - writable);
    }

    return n;
}

} // namespace tair::network
