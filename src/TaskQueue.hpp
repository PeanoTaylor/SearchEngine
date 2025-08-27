/**
 * Project 66th
 */


#pragma once

#include <iostream>
#include <functional>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
using std::cout;
using std::endl;
using std::queue;
using std::thread;
using std::mutex;
using std::unique_lock;
using std::lock_guard;
using std::condition_variable;
using std::function;
using std::bind;

extern mutex coutMtx;

using ElemType = function<void()>;

class TaskQueue {
public: 

    /**
     * @param capa
     */
    TaskQueue(size_t capa);

    ~TaskQueue();

    /**
     * @param value
     */
    //往TaskQueue中添加数据（任务）
    void push(ElemType && value);

    //从TaskQueue中获取数据（任务）
    ElemType pop();

    //判满和判空，对应的状态下
    //应该让（生产者或消费者）线程等待
    bool full();    
    bool empty();
    
    //线程池退出时唤醒所有阻塞在m_notEmpty上的子线程
    void wakeup();


private: 
    size_t m_capacity;//容量
    queue<ElemType> m_que;//存放数据的数据结构
    mutex m_mtx;//TaskQueue中的数据是共享资源，需要锁管理
    condition_variable m_notEmpty;//非空条件变量
    condition_variable m_notFull;//非满条件变量
    
    //添加一个标志位，参与到pop的判断中
    //为了让线程池退出时，子线程能够不再阻塞
    bool m_flag = true;                
};

