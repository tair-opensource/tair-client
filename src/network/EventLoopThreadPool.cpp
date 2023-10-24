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
#include "network/EventLoopThreadPool.hpp"

#include <sstream>

#include "common/ClockTime.hpp"
#include "common/Logger.hpp"
#include "network/NetworkStat.hpp"

namespace tair::network {

using common::ClockTime;

EventLoopThreadPool::EventLoopThreadPool(EventLoop *base_loop, size_t thread_num, std::string name)
    : base_loop_(base_loop), next_loop_index_(0) {
    runtimeAssert(base_loop_ != nullptr);
    runtimeAssert(thread_num >= 0);
    WriteLockGuard wlock(rw_lock_);
    thread_num_ = available_thread_num_ = thread_num;
    if (!name.empty()) {
        name_ = name;
    } else {
        std::stringstream ss;
        ss << "loop-thp-" << this;
        name_ = ss.str();
    }
}

EventLoopThreadPool::~EventLoopThreadPool() {
    stop();
    join();
}

void EventLoopThreadPool::createLoopThread(int idx) {
    std::string thread_name = name_ + "-loop-" + std::to_string(idx);
    std::string loop_name = name_ + "-loop-" + std::to_string(idx);
    auto loop_thread = std::make_shared<EventLoopThread>(thread_name, loop_name, idx);
    loop_thread->setLoopInitCallback(init_callback_);
    loop_thread->setLoopDestroyCallback(destroy_callback_);
    loop_thread->setBeforeSleepCallBack(before_sleep_call_back_);
    loop_thread->setAfterSleepCallBack(after_sleep_call_back_);
    loop_thread->start();
    threads_[idx] = loop_thread;
    loops_[idx] = loop_thread->loop();
}

void EventLoopThreadPool::start() {
    WriteLockGuard wlock(rw_lock_);
    if (stopped_) {
        LOG_DEBUG("EventLoopThreadPool start, thread_num={}, name: {}", thread_num_, name_);
        runtimeAssert(threads_.empty());
        stopped_ = false;
        threads_.resize(thread_num_, nullptr);
        loops_.resize(thread_num_, nullptr);
        for (size_t i = 0; i < thread_num_; ++i) {
            createLoopThread(i);
        }
    }
}

void EventLoopThreadPool::stop() {
    WriteLockGuard wlock(rw_lock_);
    if (!stopped_) {
        LOG_DEBUG("EventLoopThreadPool stop, thread_num={}, name: {}", thread_num_, name_);
        stopped_ = true;
        for (auto &thread : threads_) {
            if (thread) {
                thread->stop();
            }
        }
        loops_.clear();
    }
    available_thread_num_ = thread_num_ = 0;
}

void EventLoopThreadPool::join() {
    ReadLockGuard rlock(rw_lock_);
    for (auto &thread : threads_) {
        if (thread) {
            thread->join();
        }
    }
    threads_.clear();
}

bool EventLoopThreadPool::resizeThreadPoolSize(size_t new_thread_num) {
    WriteLockGuard wlock(rw_lock_);
    if (new_thread_num > MAX_THREAD_POOL_SIZE) {
        LOG_ERROR("resize thread pool size, but new_thread_num[{}] > MAX_THREAD_POOL_SIZE[{}]", new_thread_num, MAX_THREAD_POOL_SIZE);
        return false;
    }
    LOG_INFO("resize thread pool size, curr thread_num is {}, new_thread_num is {}", thread_num_, new_thread_num);
    if (new_thread_num > thread_num_) {
        // need create new loop threads
        threads_.resize(new_thread_num, nullptr);
        loops_.resize(new_thread_num, nullptr);
        for (size_t i = thread_num_; i < new_thread_num; ++i) {
            createLoopThread(i);
        }
        thread_num_ = available_thread_num_ = new_thread_num;
    } else {
        available_thread_num_ = new_thread_num;
        if (available_thread_num_ != thread_num_) {
            checkAndStopLoopThread();
        }
    }
    return true;
}

void EventLoopThreadPool::checkAndStopLoopThread() {
    if (available_thread_num_ < thread_num_ && NetworkStat::getMovingTcpConnCount() == 0) {
        size_t new_thread_num = thread_num_;
        for (size_t i = thread_num_ - 1; i >= available_thread_num_; --i) {
            if (thread_exit_check_callback_ && thread_exit_check_callback_(i, loops_[i])) {
                threads_[i]->stop();
                threads_[i]->join();
                threads_[i].reset();
                loops_[i] = nullptr;
                new_thread_num = i;
            } else {
                break;
            }
        }
        runtimeAssert(thread_num_ >= available_thread_num_);
        if (new_thread_num < thread_num_) {
            thread_num_ = new_thread_num;
            threads_.resize(thread_num_);
            loops_.resize(thread_num_);
        }
    }
    if (available_thread_num_ != thread_num_) {
        base_loop_->runAfterTimer(Duration(1 * Duration::kSecond), [this](EventLoop *loop) {
            WriteLockGuard wlock(rw_lock_);
            checkAndStopLoopThread();
        });
    }
}

void EventLoopThreadPool::runInNextlLoop(const LoopTaskCallback &callback) {
    ReadLockGuard rlock(rw_lock_);
    if (stopped_ || available_thread_num_ == 0) {
        base_loop_->runInLoop(callback);
        return;
    }
    size_t index = next_loop_index_.fetch_add(1) % available_thread_num_;
    loops_[index]->runInLoop(callback);
}

void EventLoopThreadPool::runInLoopByHash(size_t hash, const LoopTaskCallback &callback) {
    ReadLockGuard rlock(rw_lock_);
    if (stopped_ || available_thread_num_ == 0) {
        base_loop_->runInLoop(callback);
        return;
    }
    size_t index = hash % available_thread_num_;
    loops_[index]->runInLoop(callback);
}

size_t EventLoopThreadPool::runInAllLoop(const LoopTaskCallback &callback) {
    ReadLockGuard rlock(rw_lock_);
    if (stopped_ || thread_num_ == 0) {
        base_loop_->runInLoop(callback);
        return 1;
    }
    bool delay_self = false;
    for (size_t i = 0; i < thread_num_; ++i) {
        if (loops_[i] != EventLoop::getSelfLoop()) {
            loops_[i]->queueInLoop(callback);
        } else {
            delay_self = true;
        }
    }
    // if myself is one of event loop thread, call it at last
    if (delay_self) {
        callback(EventLoop::getSelfLoop());
    }
    return loops_.size();
}

size_t EventLoopThreadPool::queueInAllLoop(const LoopTaskCallback &callback) {
    ReadLockGuard rlock(rw_lock_);
    if (stopped_ || thread_num_ == 0) {
        base_loop_->queueInLoop(callback);
        return 1;
    }
    for (size_t i = 0; i < thread_num_; ++i) {
        loops_[i]->queueInLoop(callback);
    }
    return loops_.size();
}

size_t EventLoopThreadPool::runInChoosedLoop(const ChooseLoopCallback &choose_callback, const LoopTaskCallback &callback) {
    ReadLockGuard rlock(rw_lock_);
    // cannot run in base io
    if (unlikely(stopped_)) {
        return 0;
    }
    size_t choosed_count = 0;
    for (size_t i = 0; i < thread_num_; ++i) {
        if (choose_callback(loops_[i])) {
            loops_[i]->queueInLoop(callback);
            choosed_count++;
        }
    }
    return choosed_count;
}

size_t EventLoopThreadPool::runInRandomLoop(const LoopTaskCallback &callback) {
    ReadLockGuard rlock(rw_lock_);
    if (stopped_ || thread_num_ == 0) {
        base_loop_->runInLoop(callback);
        return 1;
    }
    size_t index = ClockTime::intervalNs() % thread_num_;
    loops_[index]->runInLoop(callback);
    return 1;
}

void EventLoopThreadPool::runWithAllLoop(const AllLoopTaskCallback &callback) {
    ReadLockGuard rlock(rw_lock_);
    if (unlikely(stopped_)) {
        return;
    }
    runtimeAssert(loops_.size() == thread_num_);
    callback(loops_, thread_num_, available_thread_num_);
}

} // namespace tair::network
