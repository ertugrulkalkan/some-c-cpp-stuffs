#include <iostream>
#include <thread>
#include <chrono>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <vector>
#include <random>
#include <atomic>

int thread_number = 1;

const int MAX_THREADS = std::thread::hardware_concurrency();

static std::queue<int> data = std::queue<int>();

// thread pool
static std::vector<std::thread*> thread_pool;
static std::mutex thread_pool_mutex;
static std::condition_variable thread_pool_cv;
static std::atomic<bool> thread_pool_done;

void sleep_for_a_variable_time() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1000, 10000);
    std::this_thread::sleep_for(std::chrono::milliseconds(dis(gen)));
}

void worker_thread(int number) {
    while (true) {
        std::unique_lock<std::mutex> lock(thread_pool_mutex);
        thread_pool_cv.wait(lock, [] { return !data.empty() || thread_pool_done.load(); });
        if (thread_pool_done.load()) {
            break;
        }
        int item = data.front();
        data.pop();
        lock.unlock();
        std::cout << "Thread " << number << ": " << item << " start" << std::endl;
        sleep_for_a_variable_time();
        std::cout << "Thread " << number << ": " << item << " done" << std::endl;
    }
}

int main()
{
    // create data
    for (int i = 0; i < 1000; i++)
    {
        data.push(i);
    }

    // create thread pool
    thread_pool_done.store(false);

    while(thread_pool.size() < MAX_THREADS)
    {
        thread_pool.push_back(new std::thread(worker_thread, thread_number++));
    }

    // wait for all threads to finish
    while(!data.empty())
    {
        std::unique_lock<std::mutex> lock(thread_pool_mutex);
        thread_pool_cv.notify_all();
    }

    // set done flag
    thread_pool_done.store(true);
    thread_pool_cv.notify_all();

    // join and erase threads
    for (int i = 0; i < MAX_THREADS; i++) {
        if(thread_pool[i]->joinable()) {
            thread_pool[i]->join();
            thread_pool.erase(thread_pool.begin() + i);
            --i;
        }
    }
    
    // return
    return 0;
}