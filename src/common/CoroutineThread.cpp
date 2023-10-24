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
#include "common/CoroutineThread.hpp"

#include "common/ClockTime.hpp"
#include "common/Logger.hpp"
#include "common/SystemUtil.hpp"
#include "common/libaco/aco.h"

namespace tair::common {

thread_local CoroutineThread *CoroutineThread::this_coroutine_thread_ = nullptr;

CoroutineThread::CoroutineThread(bool use_shared_stack, const std::string &name)
    : use_shared_stack_(use_shared_stack), name_(name) {
}

CoroutineThread::~CoroutineThread() {
    stop();
}

void CoroutineThread::start() {
    if (stopped_) {
        stopped_ = false;
        thread_ = std::make_unique<std::thread>(&CoroutineThread::threadFunc, this);
    }
}

void CoroutineThread::stop() {
    if (!stopped_) {
        stopped_ = true;
        not_empty_cond_.notifyAll();
        thread_->join();
    }
    if (shared_stack_) {
        aco_share_stack_destroy(shared_stack_);
        shared_stack_ = nullptr;
    }
    if (main_co_) {
        aco_destroy(main_co_);
        main_co_ = nullptr;
    }
}

void CoroutineThread::initMainCoroutine() {
    aco_thread_init(nullptr);
    main_co_ = aco_create(nullptr, nullptr, 0, nullptr, nullptr);
    if (use_shared_stack_) {
        shared_stack_ = aco_share_stack_new(0);
    }
    this_coroutine_thread_ = this;
}

void CoroutineThread::createCoroutine(Task *ptask) {
    auto co_entry = []() {
        Task *ptask = (Task *)aco_get_arg();
        (*ptask)();
        delete ptask;
        aco_exit();
    };
    aco_share_stack_t *stack = nullptr;
    if (use_shared_stack_) {
        stack = shared_stack_;
    } else {
        stack = aco_share_stack_new(0);
    }
    aco_t *co = aco_create(main_co_, stack, 0, co_entry, ptask);
    coroutines_size_++;
    coroutines_.emplace_front(co);
    LOG_TRACE("create new coroutine: {}", (void *)co);
    registerCoroutine(co);
}

void CoroutineThread::resumeCoroutine() {
    if (coroutines_.empty()) {
        return;
    }
    aco_t *co = coroutines_.front();
    coroutines_.pop_front();
    aco_resume(co);
    if (co->is_end) {
        LOG_TRACE("destroy coroutine: {}, max save stack size: {}, save count: {}, restore count: {}",
                  (void *)co, co->save_stack.max_cpsz, co->save_stack.ct_save, co->save_stack.ct_restore);
        destroyCoroutine(co);
    } else {
        coroutines_.emplace_back(co);
    }
}

void CoroutineThread::destroyCoroutine(aco_t *co) {
    unregisterCoroutine(co);
    aco_share_stack_t *stack = co->share_stack;
    aco_destroy(co);
    if (!use_shared_stack_) {
        aco_share_stack_destroy(stack);
    }
    coroutines_size_--;
}

void CoroutineThread::registerCoroutine(aco_t *co) {
    LockGuard lock(co_time_mutex_);
    auto iter = co_start_times_.emplace(ClockTime::nowMs());
    co_start_times_map_.emplace(std::make_pair(co, iter));
}

void CoroutineThread::unregisterCoroutine(aco_t *co) {
    LockGuard lock(co_time_mutex_);
    auto iter = co_start_times_map_.find(co);
    runtimeAssert(iter != co_start_times_map_.end());
    co_start_times_.erase(iter->second);
    co_start_times_map_.erase(iter);
}

int64_t CoroutineThread::getMinCoroutinesStartTime() const {
    LockGuard lock(co_time_mutex_);
    return !co_start_times_.empty() ? *co_start_times_.begin() : ClockTime::nowMs();
}

void CoroutineThread::yield() {
    runtimeAssert(this_coroutine_thread_ != nullptr);
    aco_yield();
}

int64_t CoroutineThread::getSelfMinCoroutinesStartTime() {
    runtimeAssert(this_coroutine_thread_ != nullptr);
    return this_coroutine_thread_->getMinCoroutinesStartTime();
}

CoroutineThread::Task CoroutineThread::take() {
    UniqueLock lock(mutex_);
    while (queue_.empty() && !stopped_ && coroutines_.empty()) {
        if (idle_callback_) {
            idle_callback_();
        }
        not_empty_cond_.waitFor(lock, std::chrono::milliseconds(sleep_time_ms_));
    }
    if (queue_.empty() || (max_coroutine_size_ && coroutines_size_ > max_coroutine_size_)) {
        return nullptr;
    }
    Task task = queue_.front();
    queue_.pop_front();
    return task;
}

void CoroutineThread::threadFunc() {
    std::string thread_name = std::string("ct-") + name_;
    SystemUtil::setThreadName(thread_name);
    initMainCoroutine();
    if (init_callback_) {
        init_callback_();
    }
    while (!stopped_ || !coroutines_.empty()) {
        Task task(take());
        if (task) {
            createCoroutine(new Task(task));
        }
        resumeCoroutine();
    }
    LOG_DEBUG("CoroutineThread exit, name: {}", thread_name);
}

} // namespace tair::common
