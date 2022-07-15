//
// Created by 王泓哲 on 12/07/2022.
//
#include "euterpe_thread.h"
#include "../Log/log.h"
#include <iostream>
namespace euterpe{


    /// thread_local和static extern一样 用来描述变量的生命周期
    /// 变量在线程创建时生成
    /// 线程结束时被销毁
    /// 每个线程拥有其自身的对象实例
    /// 唯有声明为 thread_local 的对象拥有此存储期
    static thread_local Thread* t_thread = nullptr;
    static thread_local std::string t_thread_name = "UNKNOW";

    static auto g_logger = EUTERPE_LOG_NAME("system");

    void  Thread::test(){
        EUTERPE_LOG_INFO(g_logger)<<"test";
    };

    Thread::Thread(std::function<void()> cb,const std::string& name):m_cb(cb),m_name(name){
        if(name.empty()){
            m_name = "UNKNOW";
        }
        m_name = name;
///        　　第一个参数为指向线程标识符的指针。
///        　　第二个参数用来设置线程属性。
///        　　第三个参数是线程运行函数的起始地址。
///        　　最后一个参数是运行函数的参数。
        int rt = pthread_create(&m_thread,nullptr,&Thread::run, this);
        if(rt) {
            EUTERPE_LOG_ERROR(g_logger) << "pThread_create error" << rt << "name = " << name;
            throw std::logic_error("pthread_create error");
        }
        m_semaphore.wait();
    };

    Thread::~Thread(){
        if(m_thread){
            pthread_detach(m_thread);
        }
    };

    void* Thread::run(void* arg){
        /// 将thread本身作为变量传入
        /// 然后给两个static类型的变量赋值
        /// 换句话说 这两个变量只有在join后才能有值
        /// 包括my_id也是在join之后才有值
        Thread* thread = (Thread*)arg;
        t_thread = thread;
        t_thread_name = thread->m_name;
        thread->m_id = euterpe::GetThreadId();
        pthread_setname_np(pthread_self(),thread->m_name.substr(0, 15).c_str());

        ///防止函数中有智能指针 引用不被释放
        std::function<void()> cb;
        cb.swap(thread->m_cb);
        thread->m_semaphore.notify();
        cb();
        return 0;
    };

    void Thread::join() {
        if(m_thread) {
            /// std::cout << m_thread <<std::endl;
            int rt = pthread_join(m_thread, nullptr);
            /// 成功执行完 返回0
            if(rt) {
                EUTERPE_LOG_ERROR(g_logger) << "pthread_join thread fail, rt=" << rt
                << " name=" << m_name;
                throw std::logic_error("pthread_join error");
            }
            m_thread = 0;
        }
    }

    Thread* Thread::GetThis(){
        return t_thread;
    };

    const std::string& Thread::GetName(){
        return t_thread_name;
    };

    void Thread::SetName(const std::string& name){
        if(name.empty()) {
            return;
        }
        if(t_thread){
            t_thread->m_name = name;
        }
        t_thread_name = name;
    }
}

