#pragma once

extern "C" {
    #include <libavformat/avformat.h>
	#include <libavcodec/avcodec.h>
}

#include "AVMemory.h"

#include <queue>
#include <mutex>
#include <condition_variable>
#include <memory>
#include <type_traits>

template<typename T>
class Channel
{
    private:
        std::queue<T> queue;
        std::mutex mutex;
        std::condition_variable not_full;
        std::condition_variable not_empty;

        size_t capacity;

        bool is_stop = false;

    public:
        Channel()
        {
            if constexpr (std::is_same_v<T, AVPacketPtr>)
            {
                capacity = -1;
            }
            else if constexpr (std::is_same_v<T, AVFramePtr>)
            {
                capacity = 50;
            }
        }
        ~Channel() = default;

        void push(T data) {
            std::unique_lock<std::mutex> lock(mutex);
            if(capacity != -1 && queue.size() >= capacity)
            {
                not_full.wait(lock, [this]{ return is_stop ||queue.size() < capacity; });
            }
            queue.push(std::move(data));
            not_empty.notify_one();
        }

        T pop() {
            std::unique_lock<std::mutex> lock(mutex);
            not_empty.wait(lock, [this]{ return is_stop || !queue.empty(); });
            T data = std::move(queue.front());
            queue.pop();
            not_full.notify_one();
            return data;
        }

        bool empty() {
            std::lock_guard<std::mutex> lock(mutex);
            return queue.empty();
        }

        size_t size() {
            std::lock_guard<std::mutex> lock(mutex);
            return queue.size();
        }

        void stop()
        {
            is_stop = true;
        }

};
