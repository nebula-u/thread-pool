#include "threadpool.h"

const uint TASK_MAX_THRESHOLD = 1024;

ThreadPool::ThreadPool()
    : initThreadSize_(4),
      taskSize_(0),
      taskQueueThreshHold_(TASK_MAX_THRESHOLD),
      poolMode_(PoolMode::MODE_FIXED)
{}

ThreadPool::~ThreadPool()
{}

void ThreadPool::setMode(PoolMode mode)
{
    poolMode_ = mode;
}

void ThreadPool::setTaskQueMaxThreshHold(uint threshold)
{
    taskQueueThreshHold_ = threshold;
}

void ThreadPool::submitTask(std::shared_ptr<Task> sp)
{

}

void ThreadPool::start(size_t size)
{
    initThreadSize_ = size;

    // 创建线程对象
    for(int i = 0; i < initThreadSize_; i++)
    {
        threads_.emplace_back(new Thread(std::bind(&ThreadPool::threadHandler, this)));
    }

    // 启动线程
    for(int i = 0; i < initThreadSize_; i++)
    {
        threads_[i]->start();
    }
}

void ThreadPool::threadHandler()
{
    std::cout << "thread start, uid: " << std::this_thread::get_id() << std::endl;
    std::cout << "thread end, uid: " << std::this_thread::get_id() << std::endl;
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