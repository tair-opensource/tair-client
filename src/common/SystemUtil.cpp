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
#include "common/SystemUtil.hpp"

#include "common/Logger.hpp"

#if defined(__APPLE__)
#include <mach-o/dyld.h>
#include <sys/sysctl.h>
#elif defined(__linux__)
#include <errno.h>
#include <linux/limits.h>
#include <sched.h>
#include <sys/prctl.h>
#include <sys/sysinfo.h>
#include <sys/wait.h>
#endif

#include <cerrno>
#include <cstdarg>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <signal.h>
#include <sys/statvfs.h>
#include <sys/utsname.h>
#include <unistd.h>

extern char **environ;

namespace tair::common {

int SystemUtil::getPid() {
    return ::getpid();
}

int SystemUtil::getNprocs() {
    int core_count;
#ifdef __APPLE__
    size_t len = sizeof(core_count);
    ::sysctlbyname("machdep.cpu.core_count", &core_count, &len, 0, 0);
#elif __linux__
    core_count = ::get_nprocs();
#endif
    return core_count;
}

std::string SystemUtil::getUnameInfo() {
    struct utsname uts;
    char uname_buff[sizeof(uts) + 3];
    if (::uname(&uts) < 0) {
        return std::string(SystemUtil::errnoToString(errno));
    }
    ::snprintf(uname_buff, sizeof(uname_buff), "%s %s %s %s", uts.sysname, uts.release, uts.version, uts.machine);
    return std::string(uname_buff);
}

std::string SystemUtil::getSelfExeFilePath() {
    char path[PATH_MAX] = {0};
    ::memset(path, 0, sizeof(path));
#ifdef __APPLE__
    uint32_t size = PATH_MAX;
    ::_NSGetExecutablePath(path, &size);
#elif __linux__
    ::readlink("/proc/self/exe", path, sizeof(path));
#endif
    return std::string(path);
}

void SystemUtil::setThreadName(const std::string &name) {
    constexpr const int maxlen = 15;
    char fullname[maxlen + 1];

    const char *cname = name.c_str();
    if (name.size() > maxlen) {
        ::strncpy(fullname, cname, maxlen);
        fullname[maxlen] = '\0';
        cname = fullname;
    }

#if defined(__APPLE__)
    ::pthread_setname_np(cname);
#elif defined(__linux__)
    ::pthread_setname_np(pthread_self(), cname);
#endif
}

#if (defined __linux || defined __APPLE__)

#ifndef SPT_MAXTITLE
#define SPT_MAXTITLE 255
#endif

static struct {
    // original value
    const char *arg0;

    // title space available
    char *base, *end;

    // pointer to original nul character within base
    char *nul;

    bool reset;
    int error;
} SPT;

static inline size_t sptMin(size_t a, size_t b) {
    return a < b ? a : b;
}

ThreadID SystemUtil::getSelfThreadID() {
    return ::pthread_self();
}

bool SystemUtil::sendSignalToThread(ThreadID id, int sig) {
    return ::pthread_kill(id, sig) == 0;
}

bool SystemUtil::sendSignalToProcess(pid_t pid, int sig) {
    return ::kill(pid, sig) == 0;
}

pid_t SystemUtil::waitForChildProcess(bool nonblocking, int &exitcode, int &bysignal) {
    int statloc;
    pid_t pid = ::wait3(&statloc, nonblocking ? WNOHANG : 0, nullptr);
    if (pid != 0) {
        exitcode = WEXITSTATUS(statloc);
        bysignal = false;
        if (WIFSIGNALED(statloc)) {
            bysignal = WTERMSIG(statloc);
        }
    }
    return pid;
}

void SystemUtil::blockSigIntAndSigTerm() {
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGINT);
    sigaddset(&mask, SIGTERM);
    if (pthread_sigmask(SIG_BLOCK, &mask, nullptr) != 0) {
        LOG_ERROR("pthread_sigmask error: {}", SystemUtil::errnoToString(errno));
    }
}

void SystemUtil::unblockSigIntAndSigTerm() {
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGINT);
    sigaddset(&mask, SIGTERM);
    if (pthread_sigmask(SIG_UNBLOCK, &mask, nullptr) != 0) {
        LOG_ERROR("pthread_sigmask error: {}", SystemUtil::errnoToString(errno));
    }
}

void SystemUtil::blockSigUsr2() {
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGUSR2);
    if (pthread_sigmask(SIG_BLOCK, &mask, nullptr) != 0) {
        LOG_ERROR("pthread_sigmask error: {}", SystemUtil::errnoToString(errno));
    }
}

void SystemUtil::unblockSigUsr2() {
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGUSR2);
    if (pthread_sigmask(SIG_UNBLOCK, &mask, nullptr) != 0) {
        LOG_ERROR("pthread_sigmask error: {}", SystemUtil::errnoToString(errno));
    }
}

int SystemUtil::getDiskUsage(const std::string &path) {
    if (path[0] != '/') {
        return -1;
    }
    struct statvfs buf;
    if (::statvfs(path.c_str(), &buf) != 0) {
        LOG_ERROR("statvfs {} error: {}", path, SystemUtil::errnoToString(errno));
        return -1;
    }
    int ret = (100.0 * (double)(buf.f_blocks - buf.f_bfree) / (double)(buf.f_blocks - buf.f_bfree + buf.f_bavail));
    return ret > 0 ? ret : 0;
}

std::string SystemUtil::errnoToString(int err) {
    char errstr[256];
    errstr[0] = '\0';
    const char *result = errstr;
#if __GLIBC__
    result = ::strerror_r(err, errstr, sizeof(errstr) - 1);
#else
    ::strerror_r(err, errstr, sizeof(errstr) - 1);
#endif
    return std::string(result);
}

void SystemUtil::exitOnFailure() {
    LOG_ERROR("exitOnFailure");
    Logger::instance().flushAllLogAndWait();
    ::_exit(EXIT_FAILURE);
}

#ifdef __GLIBC__
#define HAVE_CLEARENV
#endif

// For discussion on the portability of the various methods, see
// http://lists.freebsd.org/pipermail/freebsd-stable/2008-June/043136.html
static int sptClearenv(void) {
#ifdef HAVE_CLEARENV
    return ::clearenv();
#else
    static char **tmp;
    if (!(tmp = (char **)::malloc(sizeof *tmp)))
        return errno;
    tmp[0] = NULL;
    ::environ = tmp;
    return 0;
#endif
}

static int sptCopyenv(int envc, char **oldenv) {
    char **envcopy = NULL;
    char *eq;
    int i, error;
    int envsize;

    if (::environ != oldenv) {
        return 0;
    }
    // Copy environ into envcopy before clearing it.
    // Shallow copy is enough as clearenv() only clears the environ array.
    envsize = (envc + 1) * sizeof(char *);
    envcopy = (char **)::malloc(envsize);
    if (!envcopy) {
        return ENOMEM;
    }
    ::memcpy(envcopy, oldenv, envsize);

    // Note that the state after clearenv() failure is undefined,
    // but we'll just assume an error means it was left unchanged.
    if ((error = sptClearenv())) {
        ::environ = oldenv;
        free(envcopy);
        return error;
    }

    // Set environ from envcopy
    for (i = 0; envcopy[i]; i++) {
        if (!(eq = strchr(envcopy[i], '='))) {
            continue;
        }
        *eq = '\0';
        error = (0 != setenv(envcopy[i], eq + 1, 1)) ? errno : 0;
        *eq = '=';
        // On error, do our best to restore state
        if (error) {
#ifdef HAVE_CLEARENV
            // We don't assume it is safe to free environ, so we may leak it.
            //  As clearenv() was shallow using envcopy here is safe.
            environ = envcopy;
#else
            ::free(envcopy);
            ::free(::environ); // Safe to free, we have just alloc'd it
            ::environ = oldenv;
#endif
            return error;
        }
    }
    ::free(envcopy);

    return 0;
}

static int sptCopyargs(int argc, char **argv) {
    char *tmp;
    for (int i = 1; i < argc || (i >= argc && argv[i]); i++) {
        if (!argv[i])
            continue;
        if (!(tmp = ::strdup(argv[i])))
            return errno;
        argv[i] = tmp;
    }
    return 0;
}

// Initialize and populate SPT to allow a future setproctitle() call.
// As setproctitle() basically needs to overwrite argv[0],
// we're trying to determine what is the largest contiguous block starting at argv[0] we can use for this purpose.
//
// As this range will overwrite some or all of the argv and environ strings, a deep copy of these two arrays is performed.
void SystemUtil::sptInit(int argc, char **argv) {
    char **envp = environ;
    char *base, *end, *nul, *tmp;
    int i, error, envc;

    if (!(base = argv[0])) {
        return;
    }
    // We start with end pointing at the end of argv[0]
    nul = &base[strlen(base)];
    end = nul + 1;

    // Attempt to extend end as far as we can, while making sure that the range between base and end is only allocated to argv,
    // or anything that immediately follows argv (presumably envp).
    for (i = 0; i < argc || (i >= argc && argv[i]); i++) {
        if (!argv[i] || argv[i] < end) {
            continue;
        }
        if (end >= argv[i] && end <= argv[i] + ::strlen(argv[i])) {
            end = argv[i] + ::strlen(argv[i]) + 1;
        }
    }

    // In case the envp array was not an immediate extension to argv, scan it explicitly.
    for (i = 0; envp[i]; i++) {
        if (envp[i] < end) {
            continue;
        }
        if (end >= envp[i] && end <= envp[i] + ::strlen(envp[i])) {
            end = envp[i] + ::strlen(envp[i]) + 1;
        }
    }
    envc = i;

    // We're going to deep copy argv[], but argv[0] will still point to the old memory for the purpose
    // of updating the title so we need to keep the original value elsewhere.
    if (!(SPT.arg0 = ::strdup(argv[0]))) {
        goto syerr;
    }

#if __GLIBC__
    if (!(tmp = ::strdup(program_invocation_name))) {
        goto syerr;
    }
    program_invocation_name = tmp;
    if (!(tmp = ::strdup(program_invocation_short_name))) {
        goto syerr;
    }
    program_invocation_short_name = tmp;
#elif __APPLE__
    if (!(tmp = ::strdup(getprogname()))) {
        goto syerr;
    }
    ::setprogname(tmp);
#endif

    // Now make a full deep copy of the environment and argv[]
    if ((error = sptCopyenv(envc, envp))) {
        goto error;
    }
    if ((error = sptCopyargs(argc, argv))) {
        goto error;
    }
    SPT.nul = nul;
    SPT.base = base;
    SPT.end = end;

    return;
syerr:
    error = errno;
error:
    SPT.error = error;
}

void SystemUtil::setProcTitle(const char *fmt, ...) {
    char buf[SPT_MAXTITLE + 1]; // use buffer in case argv[0] is passed
    va_list ap;
    char *nul;

    if (!SPT.base || !fmt) {
        return;
    }
    va_start(ap, fmt);
    int len = ::vsnprintf(buf, sizeof buf, fmt, ap);
    va_end(ap);

    int error;
    if (len <= 0) {
        error = errno;
        goto error;
    }
    if (!SPT.reset) {
        ::memset(SPT.base, 0, SPT.end - SPT.base);
        SPT.reset = true;
    } else {
        ::memset(SPT.base, 0, sptMin(sizeof buf, SPT.end - SPT.base));
    }
    len = sptMin(len, sptMin(sizeof buf, SPT.end - SPT.base) - 1);
    ::memcpy(SPT.base, buf, len);
    nul = &SPT.base[len];

    if (nul < SPT.nul) {
        *SPT.nul = '.';
    } else if (nul == SPT.nul && &nul[1] < SPT.end) {
        *SPT.nul = ' ';
        *++nul = '\0';
    }
    return;

error:
    SPT.error = error;
}

#else
void SystemUtil::setproctitle(const char *fmt, ...) {
}
#endif // __linux || __APPLE__

} // namespace tair::common
