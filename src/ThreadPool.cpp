/**
 * Project 66th
 */


#include "ThreadPool.hpp"
#include <unistd.h>

/**
 * ThreadPool implementation
 */


/**
 * @param threadNum
 * @param queSize
 */
ThreadPool::ThreadPool(size_t threadNum, size_t queSize) 
: m_threadNum(threadNum)
, m_threads()
, m_queSize(queSize)
, m_que(m_queSize)
, m_isExit(false)
{

}

ThreadPool::~ThreadPool() {

}

/**
 * @return void
 */
//线程池启动
void ThreadPool::start() {
    for(size_t idx = 0; idx < m_threadNum; ++idx)
    {
        //线程池启动后，需要创建m_threadNum个子线程备用
        //但是往vector中进行push_back时不能传左值thread
        //因为这样会触发thread的拷贝构造，
        //而thread拷贝构造被删除
        //
        /* thread th(&ThreadPool::doTask,this); */
        /* m_threads.push_back(std::move(th)); */

        m_threads.push_back(thread(&ThreadPool::doTask,this));
    }
}

/**
 * @return void
*/
//关闭线程池
void ThreadPool::stop() {
    //主线程执行stop函数时不应该轻易地改变标志位
    //如果发现任务队列不为空，说明还有任务没有执行完
    while(!m_que.empty())
    {
        sleep(1);
    }

    //修改标志位，表示线程池要退出
    m_isExit = true;

    //需要将threadPool声明为TaskQueue的友元类
    /* m_que.m_notEmpty.notify_all(); */
    m_que.wakeup();
    
    //让主线程等待子线程退出
    for(auto & th : m_threads)
    {
        th.join();
    }
}

/**
 * @param ptask
 * @return void
 */
void ThreadPool::addTask(ElemType && ptask) {
    //因为如果出现空指针调用虚函数的情况
    //一定会发生段错误，所以做一个检查
    if(ptask)
    {
        //生产者线程把任务往TaskQueue里面加
        //加锁
        //判满，
        //如果满就阻塞
        //如果不满就push，并且唤醒阻塞的消费者线程（子线程）
        m_que.push(std::move(ptask)); //已经实现过了！！！
    }
}

/**
 * @return ElemType
 */
//从TaskQueue中获取任务
ElemType ThreadPool::getTask() {
    return m_que.pop(); //已经实现过了！！！
}

/**
 * @return void
 */
//doTask被注册为工作线程的入口函数
void ThreadPool::doTask() {
    //只要线程池不退出，就应该让工作线程一直执行任务
    while(!m_isExit){
        //如果任务队列为空，就让工作线程阻塞
        ElemType ptask = getTask();
        
        if(ptask)
        {
            ptask();
            /* sleep(3); */
        }
        else
        {
            cout << "ptask == nullptr" << endl;
        }
    }
}
