//
// Created by hongzhe on 22-8-8.
//

#ifndef EUTERPE_FD_MANAGER_H
#define EUTERPE_FD_MANAGER_H

#include <memory>
#include "../coroutines/fiber.h"
#include "../utils/noncopyable.h"
#include "../utils/singleton.h"
#include "../utils/utils.h"

namespace euterpe {
    class FdCtx : public std::enable_shared_from_this<FdCtx> {
    public:
        typedef std::shared_ptr<FdCtx> ptr;

        FdCtx(int fd);

        ~FdCtx();

        bool isInit() const { return m_isInit; }

        bool isSocket() const { return m_isSocket; }

        bool isClose() const { return m_isClosed; }

        /**
         * @brief 设置用户主动设置非阻塞
         * @param[in] v 是否阻塞
         */
        void setUserNonblock(bool v) { m_userNonblock = v; }

        /**
         * @brief 获取是否用户主动设置的非阻塞
         */
        bool getUserNonblock() const { return m_userNonblock; }

        /**
         * @brief 设置系统非阻塞
         * @param[in] v 是否阻塞
         */
        void setSysNonblock(bool v) { m_sysNonblock = v; }

        /**
         * @brief 获取系统非阻塞
         */
        bool getSysNonblock() const { return m_sysNonblock; }

        /**
         * @brief 设置超时时间
         * @param[in] type 类型SO_RCVTIMEO(读超时), SO_SNDTIMEO(写超时)
         * @param[in] v 时间毫秒
         */
        void setTimeout(int type, uint64_t v);

        /**
         * @brief 获取超时时间
         * @param[in] type 类型SO_RCVTIMEO(读超时), SO_SNDTIMEO(写超时)
         * @return 超时时间毫秒
         */
        uint64_t getTimeout(int type);

    private:
        /**
         * @brief 初始化
         */
        bool init();

    private:
        /// 是否初始化
        bool m_isInit: 1;
        /// 是否socket
        bool m_isSocket: 1;
        /// 是否hook非阻塞
        bool m_sysNonblock: 1;
        /// 是否用户主动设置非阻塞
        bool m_userNonblock: 1;
        /// 是否关闭
        bool m_isClosed: 1;
        /// 文件句柄
        int m_fd;
        /// 读超时时间毫秒
        uint64_t m_recvTimeout;
        /// 写超时时间毫秒
        uint64_t m_sendTimeout;
    };

    /**
    * @brief 文件句柄管理类
    */
    class FdManager {
    public:
        typedef RWMutex RWMutexType;

        /**
         * @brief 无参构造函数
         */
        FdManager();

        /**
         * @brief 获取/创建文件句柄类FdCtx
         * @param[in] fd 文件句柄
         * @param[in] auto_create 是否自动创建
         * @return 返回对应文件句柄类FdCtx::ptr
         */
        FdCtx::ptr get(int fd, bool auto_create = false);

        /**
         * @brief 删除文件句柄类
         * @param[in] fd 文件句柄
         */
        void del(int fd);

    private:
        /// 读写锁
        RWMutexType m_mutex;
        /// 文件句柄集合
        std::vector<FdCtx::ptr> m_datas;
    };

    /// 文件句柄单例
    typedef Singleton<FdManager> FdMgr;

}


#endif //EUTERPE_FD_MANAGER_H
