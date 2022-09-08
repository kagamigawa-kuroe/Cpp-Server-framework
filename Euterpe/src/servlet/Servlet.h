//
// Created by hongzhe on 22-9-6.
//

/**
 * @brief Servlet的本质是一种对请求的处理
 * @brief 他包含一个回调函数m_cb 参数为一个HttpRequest 一个HttpResponse 以及一个session
 * @brief 其中的handle函数就是去执行m_cb函数 m_cb里面会进行我们的响应 最常见的就是返回html页面对应的信息
 * @brief 也就是去设置对应HttpResponse的响应体 setBody
 * @brief
 */

#ifndef EUTERPE_SERVLET_H
#define EUTERPE_SERVLET_H
#include <memory>
#include <functional>
#include <string>
#include <vector>
#include <unordered_map>
#include "../http/http.h"
#include "../http/HttpSession.h"
#include "../thread/euterpe_thread.h"
#include "../utils/utils.h"

namespace euterpe {
    namespace http {
        //////////////////////////////////////////////////////////////
        class Servlet {
        public:
            typedef std::shared_ptr<Servlet> ptr;
        public:
            Servlet(const std::string& name)
                    :m_name(name) {}

            virtual ~Servlet() {}

            virtual int32_t handle(HttpRequest::ptr request
                    , HttpResponse::ptr response
                    , HttpSession::ptr session) = 0;

            const std::string& getName() const { return m_name;}
        protected:
            /// 名称
            std::string m_name;
        };

        ////////////////////////////////////////////////////////////////
        class FunctionServlet : public Servlet {
        public:
            /// 智能指针类型定义
            typedef std::shared_ptr<FunctionServlet> ptr;
            /// 函数回调类型定义
            typedef std::function<int32_t (HttpRequest::ptr request
                    , HttpResponse::ptr response
                    , HttpSession::ptr session)> callback;


            FunctionServlet(callback cb);
            virtual int32_t handle(http::HttpRequest::ptr request
                    , HttpResponse::ptr response
                    , HttpSession::ptr session) override;
        private:
            /// 回调函数
            callback m_cb;
        };

        /////////////////////////////////////////////////////////////////
        class IServletCreator {
        public:
            typedef std::shared_ptr<IServletCreator> ptr;
            virtual ~IServletCreator() {}
            virtual Servlet::ptr get() const = 0;
            virtual std::string getName() const = 0;
        };

        ///////////////////////////////////////////////////////////////////////
        class HoldServletCreator : public IServletCreator {
        public:
            typedef std::shared_ptr<HoldServletCreator> ptr;
            HoldServletCreator(Servlet::ptr slt)
                    :m_servlet(slt) {
            }

            Servlet::ptr get() const override {
                return m_servlet;
            }

            std::string getName() const override {
                return m_servlet->getName();
            }
        private:
            Servlet::ptr m_servlet;
        };

        ////////////////////////////////////////////////////////////////////////
        template<class T>
        class ServletCreator : public IServletCreator {
        public:
            typedef std::shared_ptr<ServletCreator> ptr;

            ServletCreator() {
            }

            Servlet::ptr get() const override {
                return Servlet::ptr(new T);
            }

            std::string getName() const override {
                return TypeToName<T>();
            }
        };


        ////////////////////////////////////////////////////////////////////
        /**
         * @brief Servlet分发器
         */
        ////////////////////////////////////////////////////////////////////
        class ServletDispatch : public Servlet {
        public:
            /// 智能指针类型定义
            typedef std::shared_ptr<ServletDispatch> ptr;
            /// 读写锁类型定义
            typedef RWMutex RWMutexType;

            /**
             * @brief 构造函数
             */
            ServletDispatch();
            virtual int32_t handle(HttpRequest::ptr request
                    , HttpResponse::ptr response
                    , HttpSession::ptr session) override;

            /**
             * @brief 添加servlet
             * @param[in] uri uri
             * @param[in] slt serlvet
             */
            void addServlet(const std::string& uri, Servlet::ptr slt);

            /**
             * @brief 添加servlet
             * @param[in] uri uri
             * @param[in] cb FunctionServlet回调函数
             */
            void addServlet(const std::string& uri, FunctionServlet::callback cb);

            /**
             * @brief 添加模糊匹配servlet
             * @param[in] uri uri 模糊匹配 /sylar_*
             * @param[in] slt servlet
             */
            void addGlobServlet(const std::string& uri, Servlet::ptr slt);

            /**
             * @brief 添加模糊匹配servlet
             * @param[in] uri uri 模糊匹配 /sylar_*
             * @param[in] cb FunctionServlet回调函数
             */
            void addGlobServlet(const std::string& uri, FunctionServlet::callback cb);

            void addServletCreator(const std::string& uri, IServletCreator::ptr creator);
            void addGlobServletCreator(const std::string& uri, IServletCreator::ptr creator);

            template<class T>
            void addServletCreator(const std::string& uri) {
                addServletCreator(uri, std::make_shared<ServletCreator<T> >());
            }

            template<class T>
            void addGlobServletCreator(const std::string& uri) {
                addGlobServletCreator(uri, std::make_shared<ServletCreator<T> >());
            }

            /**
             * @brief 删除servlet
             * @param[in] uri uri
             */
            void delServlet(const std::string& uri);

            /**
             * @brief 删除模糊匹配servlet
             * @param[in] uri uri
             */
            void delGlobServlet(const std::string& uri);

            /**
             * @brief 返回默认servlet
             */
            Servlet::ptr getDefault() const { return m_default;}

            /**
             * @brief 设置默认servlet
             * @param[in] v servlet
             */
            void setDefault(Servlet::ptr v) { m_default = v;}


            /**
             * @brief 通过uri获取servlet
             * @param[in] uri uri
             * @return 返回对应的servlet
             */
            Servlet::ptr getServlet(const std::string& uri);

            /**
             * @brief 通过uri获取模糊匹配servlet
             * @param[in] uri uri
             * @return 返回对应的servlet
             */
            Servlet::ptr getGlobServlet(const std::string& uri);

            /**
             * @brief 通过uri获取servlet
             * @param[in] uri uri
             * @return 优先精准匹配,其次模糊匹配,最后返回默认
             */
            Servlet::ptr getMatchedServlet(const std::string& uri);

            void listAllServletCreator(std::map<std::string, IServletCreator::ptr>& infos);
            void listAllGlobServletCreator(std::map<std::string, IServletCreator::ptr>& infos);
        private:
            /// 读写互斥量
            RWMutexType m_mutex;
            /// 精准匹配servlet MAP
            /// uri(/sylar/xxx) -> servlet
            std::unordered_map<std::string, IServletCreator::ptr> m_datas;
            /// 模糊匹配servlet 数组
            /// uri(/sylar/*) -> servlet
            std::vector<std::pair<std::string, IServletCreator::ptr> > m_globs;
            /// 默认servlet，所有路径都没匹配到时使用
            Servlet::ptr m_default;
        };


        ////////////////////////////////////////////////////////////////////////////////////////////
        class NotFoundServlet : public Servlet {
        public:
            /// 智能指针类型定义
            typedef std::shared_ptr<NotFoundServlet> ptr;
            /**
             * @brief 构造函数
             */
            NotFoundServlet(const std::string& name);
            virtual int32_t handle(HttpRequest::ptr request
                    , HttpResponse::ptr response
                    , HttpSession::ptr session) override;

        private:
            std::string m_name;
            std::string m_content;
        };



    } // euterpe
} // http

#endif //EUTERPE_SERVLET_H
