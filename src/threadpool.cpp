#include "threadpool.h"

const uint TASK_MAX_THRESHOLD = 40; // 最大任务数量

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
Result ThreadPool::submitTask(std::shared_ptr<Task> sp)
{
    std::unique_lock<std::mutex> lock(taskQueMtx_);

    if (!notFull_.wait_for(lock,
                           std::chrono::seconds(1),
                           [&]() -> bool
                           { return taskQueue_.size() < taskQueueThreshHold_; }))
    {
        // 等待1秒钟，条件依然没有满足
        std::cerr << "task queue is full, submit task fail" << std::endl;
        return Result(sp, false);
    }

    taskQueue_.emplace(sp);
    taskSize_++;

    notEmpty_.notify_all();
    return Result(sp);
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
            notEmpty_.wait(lock, [&]() -> bool
                           { return taskQueue_.size() > 0; });

            task = taskQueue_.front();
            taskQueue_.pop();
            taskSize_--;
            if (taskQueue_.size() > 0)
            {
                notEmpty_.notify_all();
            }
            notFull_.notify_all();
        }
        if (nullptr != task)
        {
            task->exec();
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

Task::Task()
    : result_(nullptr)
{
}

void Task::exec()
{
    result_->setValue(run());
}

void Task::setResult(Result *res)
{
    result_ = res;
}

Result::Result(std::shared_ptr<Task> task, bool isValid)
    : task_(task),
      isValid_(isValid)
{
    task_->setResult(this);
}

Any Result::get()
{
    if (!isValid_)
    {
        return "";
    }
    sem_.wait(); // task任务如果没有执行完，这里会阻塞用户的线程
    return std::move(any_);
}

void Result::setValue(Any any)
{
    this->any_ = std::move(any);
    sem_.post();
}