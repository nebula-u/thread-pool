# 线程池实现

使用举例：

```C++
ThreadPool pool;
pool.setModel(fixed(default)|cached);
pool.start();

// 将任务提交至线程池
Result result = pool.submitTask(concreteTask);
```

