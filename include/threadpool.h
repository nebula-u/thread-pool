#include <vector>
#include <queue>
#include <memory>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <iostream>
#include <thread>

// 任务抽象基类
class Task
{
public:
    // 用户可自定义任意任务类型，继承自Task，重写run方法，实现自定义任务处理
    virtual void run() = 0;
};

// 线程池模式
enum class PoolMode
{
    MODE_FIXED,  // 固定数量线程
    MODE_CACHED, // 线程数量可动态增长
};

class Thread
{
public:
    using ThreadHandler = std::function<void()>;

    // 线程构造
    Thread(ThreadHandler handler);

    // 线程析构
    ~Thread();

    // 启动线程
    void start();

private:
    ThreadHandler handler_;
};

// 线程池类型
class ThreadPool
{
public:
    ThreadPool();
    ~ThreadPool();

    // 设置线程池的工作模式
    void setMode(PoolMode mode);

    // 设置Task任务队列上限阈值
    void setTaskQueMaxThreshHold(uint threshhold);

    // 给线程池提交任务
    void submitTask(std::shared_ptr<Task> sp);

    // 开启线程池
    void start(size_t size = 4);

    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator = (const ThreadPool&) = delete;

private:
    // 线程入口函数，由于线程<->任务的分配是由线程池来完成，所以将其定义在线程池类中，而不是线程类中
    void threadHandler();

private:
    std::vector<Thread *> threads_; // 线程列表
    size_t initThreadSize_;         // 初始线程数量

    std::queue<std::shared_ptr<Task>> taskQueue_; // 任务队列
                                                  // 注：在这里使用智能指针，防止Task为一个短生命周期任务导致指针为空
    std::atomic_uint taskSize_;                   // 任务数量
    uint taskQueueThreshHold_;                    // 最大任务数量阈值

    std::mutex taskQueMtx_;            // 保证任务队列的线程安全
    std::condition_variable notFull_;  // 保证任务队列不满
    std::condition_variable notEmpty_; // 保证任务队列不空

    PoolMode poolMode_; //当前线程池的工作模式
};