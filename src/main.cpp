#include "threadpool.h"
#include <unistd.h>

class MyTask : public Task
{
public:
    Any run()
    {
        int sum = 0;
        for(int i = 0; i < 10; i++)
        {
            sum += i;
        }
        return sum;
    }
};


int main(){
    ThreadPool pool;
    pool.start(4);

    Result result = pool.submitTask(std::make_shared<MyTask>());
    std::cout << result.get().cast_<int>() << std::endl;

    getchar();
    return 0;
}