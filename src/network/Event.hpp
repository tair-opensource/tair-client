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

#include <event2/buffer.h>
#include <event2/bufferevent.h>
#include <event2/dns.h>
#include <event2/dns_compat.h>
#include <event2/dns_struct.h>
#include <event2/event-config.h>
#include <event2/event.h>
#include <event2/event_compat.h>
#include <event2/event_struct.h>
#include <event2/listener.h>
#include <event2/util.h>
#include <event2/watch.h>

#ifndef H_CASE_STRING_BIGIN
#define H_CASE_STRING_BIGIN(state) switch (state) {
#define H_CASE_STRING(state) \
    case state:              \
        return #state;       \
        break;
#define H_CASE_STRING_END()           \
    default: return "Unknown"; break; \
        }
#endif

#ifndef _WIN32

#if EAGAIN == EWOULDBLOCK
#define EVUTIL_ERR_IS_EAGAIN(e) \
    ((e) == EAGAIN)
#else
#define EVUTIL_ERR_IS_EAGAIN(e) \
    ((e) == EAGAIN || (e) == EWOULDBLOCK)
#endif

/* True iff e is an error that means a read/write operation can be retried. */
#define EVUTIL_ERR_RW_RETRIABLE(e) \
    ((e) == EINTR || EVUTIL_ERR_IS_EAGAIN(e))
/* True iff e is an error that means an connect can be retried. */
#define EVUTIL_ERR_CONNECT_RETRIABLE(e) \
    ((e) == EINTR || (e) == EINPROGRESS)
/* True iff e is an error that means a accept can be retried. */
#define EVUTIL_ERR_ACCEPT_RETRIABLE(e) \
    ((e) == EINTR || EVUTIL_ERR_IS_EAGAIN(e) || (e) == ECONNABORTED)

/* True iff e is an error that means the connection was refused */
#define EVUTIL_ERR_CONNECT_REFUSED(e) \
    ((e) == ECONNREFUSED)

#else /* Win32 */

#define EVUTIL_ERR_IS_EAGAIN(e) \
    ((e) == WSAEWOULDBLOCK || (e) == EAGAIN)

#define EVUTIL_ERR_RW_RETRIABLE(e) \
    ((e) == WSAEWOULDBLOCK || (e) == WSAEINTR)

#define EVUTIL_ERR_CONNECT_RETRIABLE(e) \
    ((e) == WSAEWOULDBLOCK || (e) == WSAEINTR || (e) == WSAEINPROGRESS || (e) == WSAEINVAL)

#define EVUTIL_ERR_ACCEPT_RETRIABLE(e) \
    EVUTIL_ERR_RW_RETRIABLE(e)

#define EVUTIL_ERR_CONNECT_REFUSED(e) \
    ((e) == WSAECONNREFUSED)

#endif
