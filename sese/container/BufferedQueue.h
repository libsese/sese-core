/// \file BufferedQueue.h
/// \brief 缓存队列
/// \author kaoru
/// \date 2024年2月16日

#pragma once

#include <queue>
#include <sese/thread/Locker.h>

namespace sese {

/// 缓存队列（多生产者-多消费者）
template<class T>
class BufferedQueueNn {
public:
    explicit BufferedQueueNn(size_t write_limit) : writeLimit(write_limit) {}

    bool pop(T &t) {
        Locker locker(readMutex);
        if (readQueue.empty()) {
            writeMutex.lock();
            swap();
            writeMutex.unlock();
            if (readQueue.empty()) {
                return false;
            } else {
                t = readQueue.front();
                readQueue.pop();
                return true;
            }
        } else {
            t = readQueue.front();
            readQueue.pop();
            return true;
        }
    }

    bool push(const T &t) {
        Locker locker(writeMutex);
        if (writeQueue.size() >= writeLimit) {
            return false;
        } else {
            writeQueue.push(t);
            return true;
        }
    }

private:
    void swap() {
        std::swap(q1, q2);
    }

private:
    std::queue<T> q1;
    std::queue<T> q2;

    std::queue<T> &writeQueue = q1;
    std::queue<T> &readQueue = q2;

    std::mutex writeMutex;
    std::mutex readMutex;

    size_t writeLimit;
};

/// 缓存队列（多生产者-单消费者）
template<class T>
using BufferedQueueN1 = BufferedQueueNn<T>;

/// 缓存队列（单生产者-多消费者）
template<class T>
using BufferedQueue1N = BufferedQueueNn<T>;

/// 缓存队列（单生产者-单消费者）
template<class T>
class BufferedQueue11 {
public:
    explicit BufferedQueue11(size_t write_limit) : writeLimit(write_limit) {}

    bool pop(T &t) {
        Locker locker(mutex);
        if (queue.empty()) {
            return false;
        } else {
            t = queue.front();
            queue.pop();
            return true;
        }
    }

    bool push(const T &t) {
        Locker locker(mutex);
        if (queue.size() >= writeLimit) {
            return false;
        } else {
            queue.push(t);
            return true;
        }
    }

private:
    std::queue<T> queue;

    std::mutex mutex;

    size_t writeLimit;
};

} // namespace sese