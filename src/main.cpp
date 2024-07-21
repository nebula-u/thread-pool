#include "threadpool.h"
#include <unistd.h>

class MyTask : public Task
{
public:
    void run()
    {
        for(int i = 0; i < 10; i++)
        {
            std::cout << i << std::endl;
            sleep(1);
        }
    }
};


int main(){

    ThreadPool pool;
    pool.start(4);

    pool.submitTask(std::make_shared<MyTask>());
    pool.submitTask(std::make_shared<MyTask>());
    pool.submitTask(std::make_shared<MyTask>());
    pool.submitTask(std::make_shared<MyTask>());
    pool.submitTask(std::make_shared<MyTask>());
    pool.submitTask(std::make_shared<MyTask>());
    pool.submitTask(std::make_shared<MyTask>());
    pool.submitTask(std::make_shared<MyTask>());
    pool.submitTask(std::make_shared<MyTask>());
    pool.submitTask(std::make_shared<MyTask>());


    getchar();
    return 0;
}