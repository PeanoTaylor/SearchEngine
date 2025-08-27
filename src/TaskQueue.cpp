/**
 * Project 66th
 */


#include "TaskQueue.hpp"

/**
 * TaskQueue implementation
 */


/**
 * @param capa
 */
TaskQueue::TaskQueue(size_t capa) 
    : m_capacity(capa)
{}

TaskQueue::~TaskQueue() 
{}

/**
 * @param value
 * @return void
 */
//生产者线程会调用push函数
void TaskQueue::push(ElemType && value) {
    //1.先上锁
    unique_lock<mutex> ul(m_mtx);

    //2.判满
    /* if(full()) */
    while(full())//防止虚假唤醒
    {
        //如果TaskQueue是满的，就应该让
        //生产者在条件变量上等待
        m_notFull.wait(ul);
        //底层原理是：让生产者线程阻塞，
        //会先释放锁，等到被唤醒时再重新持有锁
    }

    //如果TaskQueue不是满的，
    //生产者线程就可以将生产的数据加入TaskQueue
    m_que.push(std::move(value));

    //唤醒消费者，可以开始消费仓库中的数据
    m_notEmpty.notify_one();

    //3.解锁（自动进行）
}

/**
 * @return int
 */
//消费者线程会调用pop函数
ElemType TaskQueue::pop() {
    unique_lock<mutex> ul(m_mtx);

    //因为此处仅仅以任务队列是否为空作为判断条件
    //所以最后线程池退出时唤醒了子线程
    //又会马上陷入阻塞
    //
    //任务队列为空，且线程池不退出，那么应该阻塞子线程
    //任务队列为空，且线程池退出，那么不应该阻塞子线程
    while(empty() && m_flag)
    {
        m_notEmpty.wait(ul);
    }

    //m_flag为true意味着线程池不退出
    //能够通过前面的循环完全是因为empty()的结果为false
    //代表着任务队列中有任务
    //才能进行取任务
    if(m_flag)
    {
        ElemType temp = m_que.front();
        m_que.pop();

        //唤醒生产者
        m_notFull.notify_one();

        return temp;
    }
    else
    {
        return nullptr;
    }
}

/**
 * @return bool
 */
bool TaskQueue::full() {
    return m_que.size() == m_capacity;
}

/**
 * @return bool
 */
bool TaskQueue::empty() {
    return m_que.size() == 0;
}

void TaskQueue::wakeup()
{
    m_flag = false;
    m_notEmpty.notify_all();
}
