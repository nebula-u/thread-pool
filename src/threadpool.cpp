#include "threadpool.h"

const uint TASK_MAX_THRESHOLD = 4;

ThreadPool::ThreadPool()
    : initThreadSize_(4),
      taskSize_(0),
      taskQueueThreshHold_(TASK_MAX_THRESHOLD),
      poolMode_(PoolMode::MODE_FIXED)
{
}

ThreadPool::~ThreadPool()
{
}

void ThreadPool::setMode(PoolMode mode)
{
    poolMode_ = mode;
}

void ThreadPool::setTaskQueMaxThreshHold(uint threshold)
{
    taskQueueThreshHold_ = threshold;
}

/**
 * 获取锁
 * 线程的通信
 */
void ThreadPool::submitTask(std::shared_ptr<Task> sp)
{
    std::unique_lock<std::mutex> lock(taskQueMtx_);

    if (!notFull_.wait_for(lock,
                           std::chrono::seconds(1),
                           [&]() -> bool
                           { return taskQueue_.size() < taskQueueThreshHold_; }))
    {
        // 等待1秒钟，条件依然没有满足
        std::cerr << "task queue is full, submit task fail" << std::endl;
        return;
    }

    taskQueue_.emplace(sp);
    taskSize_++;

    notEmpty_.notify_all();
}

void ThreadPool::start(size_t size)
{
    initThreadSize_ = size;

    // 创建线程对象
    for (int i = 0; i < initThreadSize_; i++)
    {
        auto ptr = std::make_unique<Thread>(std::bind(&ThreadPool::threadHandler, this));
        threads_.emplace_back(std::move(ptr));
    }

    // 启动线程
    for (int i = 0; i < initThreadSize_; i++)
    {
        threads_[i]->start();
    }
}

void ThreadPool::threadHandler()
{
    for (;;)
    {
        std::shared_ptr<Task> task = nullptr;
        {
            std::unique_lock<std::mutex> lock(taskQueMtx_);
            std::cout << "等待取出任务来执行" << std::endl;
            notEmpty_.wait(lock, [&]() -> bool
                           { return taskQueue_.size() > 0; });

            task = taskQueue_.front();
            taskQueue_.pop();
            taskSize_--;
            std::cout << "已经取出一条任务" << std::endl;
            if (taskQueue_.size() > 0)
            {
                notEmpty_.notify_all();
            }
            notFull_.notify_all();
        }
        if (nullptr != task)
        {
            task->run();
        }
    }
}

Thread::Thread(ThreadHandler handler)
    : handler_(handler)
{
}

Thread::~Thread()
{
}

void Thread::start()
{
    // 创建一个线程来执行线程函数
    std::thread t(handler_);

    // 设置分离线程
    t.detach();
}