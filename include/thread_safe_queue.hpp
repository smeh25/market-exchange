#pragma once

#include <condition_variable>
#include <iostream>
#include <mutex>
#include <queue>

template <typename T>
class ThreadSafeQueue {
    private:
        std::queue<T> queue;
        std::mutex mtx;
        std::condition_variable c_var;


    public:
        void push(T item){
            std::unique_lock<std::mutex> lock(mtx);

            queue.push(item);

            c_var.notify_one();
        }

        T pop(){
            std::unique_lock<std::mutex> lock(mtx);
            
            c_var.wait(lock, [this]{ return !queue.empty(); });

            T item = queue.front();
            queue.pop();

            return item;
        }

        size_t size() {
            std::unique_lock<std::mutex> lock(mtx);
            return queue.size();
        }

};
