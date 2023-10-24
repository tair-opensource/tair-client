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

#include <pthread.h>
#include <string>

using ThreadID = pthread_t;

namespace tair::common {

#define CHECK_FLAG_SLEEP(stop, interval_ms)                            \
    {                                                                  \
        int count = interval_ms;                                       \
        while (!(stop) && count-- > 0) {                               \
            std::this_thread::sleep_for(std::chrono::milliseconds(1)); \
        }                                                              \
    }

class SystemUtil {
public:
    static int getPid();
    static int getNprocs();
    static std::string getUnameInfo();
    static std::string getSelfExeFilePath();
    static void setThreadName(const std::string &name);
    static ThreadID getSelfThreadID();
    static bool sendSignalToThread(ThreadID id, int sig);
    static bool sendSignalToProcess(pid_t pid, int sig);
    static pid_t waitForChildProcess(bool nonblocking, int &exitcode, int &bysignal);
    static void blockSigIntAndSigTerm();
    static void unblockSigIntAndSigTerm();
    static void blockSigUsr2();
    static void unblockSigUsr2();
    static int getDiskUsage(const std::string &path);
    static std::string errnoToString(int err);
    static void exitOnFailure();

    static void sptInit(int argc, char **argv);
    static void setProcTitle(const char *fmt, ...);
};

} // namespace tair::common
