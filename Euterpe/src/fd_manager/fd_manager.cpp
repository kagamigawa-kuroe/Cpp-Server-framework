//
// Created by hongzhe on 22-8-8.
//

#include "fd_manager.h"
#include "../hook/hook.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

/// SO_RCVTIMEO, SO_SNDTIMEO 套接字选项 用来修改接受和发送的时间
euterpe::FdCtx::FdCtx(int fd)
        :m_isInit(false)
        ,m_isSocket(false)
        ,m_sysNonblock(false)
        ,m_userNonblock(false)
        ,m_isClosed(false)
        ,m_fd(fd)
        ,m_recvTimeout(-1)
        ,m_sendTimeout(-1) {
    init();
}

euterpe::FdCtx::~FdCtx() {

}

void euterpe::FdCtx::setTimeout(int type, uint64_t v) {
    if(type == SO_RCVTIMEO) {
        m_recvTimeout = v;
    } else {
        m_sendTimeout = v;
    }
}

uint64_t euterpe::FdCtx::getTimeout(int type) {
    if(type == SO_RCVTIMEO) {
        return m_recvTimeout;
    } else {
        return m_sendTimeout;
    }
}

bool euterpe::FdCtx::init() {
    /// 先判断是否已经初始化
    /// 是则直接跳过
    if(m_isInit) {
        return true;
    }

    /// 设置过期事件
    m_recvTimeout = -1;
    m_sendTimeout = -1;

    /// 取出fd状态 判断是否已经关闭
    struct stat fd_stat;
    if(-1 == fstat(m_fd, &fd_stat)) {
        m_isInit = false;
        m_isSocket = false;
    } else {
        /// 判断是否是一个socket连接
        m_isInit = true;
        m_isSocket = S_ISSOCK(fd_stat.st_mode);
    }

    /// 是socket则设置为非阻塞状态
    if(m_isSocket) {
        int flags = fcntl_f(m_fd, F_GETFL, 0);
        if(!(flags & O_NONBLOCK)) {
            fcntl_f(m_fd, F_SETFL, flags | O_NONBLOCK);
        }
        m_sysNonblock = true;
    } else {
        m_sysNonblock = false;
    }

    m_userNonblock = false;
    m_isClosed = false;
    return m_isInit;
}

//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////


euterpe::FdManager::FdManager() {
    m_datas.resize(64);
}

euterpe::FdCtx::ptr euterpe::FdManager::get(int fd, bool auto_create) {
    if(fd == -1) {
        return nullptr;
    }
    RWMutexType::ReadLock lock(m_mutex);
    if((int)m_datas.size() <= fd) {
        if(!auto_create) {
            return nullptr;
        }
    } else {
        if(m_datas[fd] || !auto_create) {
            return m_datas[fd];
        }
    }
    lock.unlock();

    RWMutexType::WriteLock lock2(m_mutex);
    FdCtx::ptr ctx(new FdCtx(fd));
    if(fd >= (int)m_datas.size()) {
        m_datas.resize(fd * 1.5);
    }
    m_datas[fd] = ctx;
    return ctx;
}

void euterpe::FdManager::del(int fd) {
    RWMutexType::WriteLock lock(m_mutex);
    if((int)m_datas.size() <= fd) {
        return;
    }
    m_datas[fd].reset();
}
