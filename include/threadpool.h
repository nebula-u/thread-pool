#include <vector>
#include <queue>
#include <memory>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <iostream>
#include <thread>
#include <string>

class Any
{
public:
    Any() = default;
    ~Any() = default;
    Any(const Any &) = delete;
    Any &operator=(const Any &) = delete;
    Any(Any &&) = default;
    Any &operator=(Any &&) = default;

    template <typename T>
    Any(T data) : base_(std::make_unique<Derive<T>>(data)) {}

    template <typename T>
    T cast_()
    {
        Derive<T> *pd = dynamic_cast<Derive<T>*>(base_.get());
        if (pd == nullptr)
        {
            throw "type is unmatch";
        }
        return pd->data_;
    }

private:
    class Base
    {
    public:
        virtual ~Base() = default;
    };

    template <typename T>
    class Derive : public Base
    {
    public:
        Derive(T data) : data_(data) {}
        T data_;
    };

private:
    std::unique_ptr<Base> base_;
};

class Semaphore
{
public:
    Semaphore(int limit = 0) : resLimit_(limit) {}
    ~Semaphore() = default;

    void wait()
    {
        std::unique_lock<std::mutex> lock(mtx_);
        cond_.wait(lock, [&]() -> bool
                   { return resLimit_ > 0; });
        resLimit_--;
    }

    void post()
    {
        std::unique_lock<std::mutex> lock(mtx_);
        resLimit_++;
        cond_.notify_all();
    }

private:
    int resLimit_;
    std::mutex mtx_;
    std::condition_variable cond_;
};

class Task;

class Result
{
public:
    Result(std::shared_ptr<Task> task, bool isValid = true);
    ~Result() = default;

    Any get();
    void setValue(Any any);

private:
    Any any_;                    // 存储任务返回值
    Semaphore sem_;              // 线程通信信号量
    std::shared_ptr<Task> task_; // 指向将来获取返回值的任务对象
    std::atomic_bool isValid_;   // 返回值是否有效
};

// 任务抽象基类
class Task
{
public:
    Task();
    ~Task() = default;
    void setResult(Result* res);
    void exec();

    // 需要用户重写run方法，实现自身需要的功能
    virtual Any run() = 0;

private:
    Result* result_;
};

// 线程池模式
enum class PoolMode
{
    MODE_FIXED,  // 固定数量线程（默认，缺省值为4）
    MODE_CACHED, // 线程数量可动态增长
};

class Thread
{
public:
    using ThreadHandler = std::function<void()>;

    Thread(ThreadHandler handler);

    ~Thread();

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
    Result submitTask(std::shared_ptr<Task> sp);

    // 开启线程池
    void start(size_t size = 4);

    ThreadPool(const ThreadPool &) = delete;
    ThreadPool &operator=(const ThreadPool &) = delete;

private:
    // 线程入口函数，由于线程<->任务的分配是由线程池来完成，所以将其定义在线程池类中，而不是线程类中
    void threadHandler();

private:
    std::vector<std::unique_ptr<Thread>> threads_; // 线程列表
    size_t initThreadSize_;                        // 初始线程数量

    std::queue<std::shared_ptr<Task>> taskQueue_; // 任务队列
                                                  // 注：在这里使用智能指针，防止Task为一个短生命周期任务导致指针为空
    std::atomic_uint taskSize_;                   // 当前任务数量
    size_t taskQueueThreshHold_;                  // 最大任务数量阈值

    std::mutex taskQueMtx_;            // 保证任务队列的线程安全
    std::condition_variable notFull_;  // 保证任务队列不满
    std::condition_variable notEmpty_; // 保证任务队列不空

    PoolMode poolMode_; // 当前线程池的工作模式
};