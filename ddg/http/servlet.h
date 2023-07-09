#ifndef DDG_SERVLET_H_
#define DDG_SERVLET_H_

#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include "ddg/http/http.h"
#include "ddg/http/http_session.h"
#include "ddg/thread.h"
#include "ddg/utils.h"

namespace ddg {
namespace http {

/**
 * @brief Servlet封装
 */
class Servlet {
 public:
  using ptr = std::shared_ptr<Servlet>;

  Servlet(const std::string& name);

  virtual ~Servlet();

  virtual int32_t handle(ddg::http::HttpRequest::ptr request,
                         ddg::http::HttpResponse::ptr response,
                         ddg::http::HttpSession::ptr session) = 0;

  const std::string& getName() const;

 protected:
  /// 名称
  std::string m_name;
};

/**
 * @brief 函数式Servlet
 */
class FunctionServlet : public Servlet {
 public:
  /// 智能指针类型定义
  using ptr = std::shared_ptr<FunctionServlet>;
  /// 函数回调类型定义
  using callback = std::function<int32_t(ddg::http::HttpRequest::ptr request,
                                         ddg::http::HttpResponse::ptr response,
                                         ddg::http::HttpSession::ptr session)>;

  /**
     * @brief 构造函数
     * @param[in] cb 回调函数
     */
  FunctionServlet(callback cb);
  virtual int32_t handle(ddg::http::HttpRequest::ptr request,
                         ddg::http::HttpResponse::ptr response,
                         ddg::http::HttpSession::ptr session) override;

 private:
  /// 回调函数
  callback m_cb;
};

class IServletCreator {
 public:
  typedef std::shared_ptr<IServletCreator> ptr;

  virtual ~IServletCreator();

  virtual Servlet::ptr get() const = 0;
  virtual const std::string& getName() const = 0;
};

class HoldServletCreator : public IServletCreator {
 public:
  typedef std::shared_ptr<HoldServletCreator> ptr;

  HoldServletCreator(Servlet::ptr slt);

  Servlet::ptr get() const override;

  const std::string& getName() const override;

 private:
  Servlet::ptr m_servlet;
};

template <class T>
class ServletCreator : public IServletCreator {
 public:
  using ptr = std::shared_ptr<ServletCreator>;

  ServletCreator(){};

  Servlet::ptr get() const override { return Servlet::ptr(new T); }

  const std::string& getName() const override;
};

/**
 * @brief Servlet分发器
 */
class ServletDispatch : public Servlet {
 public:
  using ptr = std::shared_ptr<ServletDispatch>;

  using RWMutexType = RWMutex;

  ServletDispatch();
  virtual int32_t handle(ddg::http::HttpRequest::ptr request,
                         ddg::http::HttpResponse::ptr response,
                         ddg::http::HttpSession::ptr session) override;

  void addServlet(const std::string& uri, Servlet::ptr slt);

  void addServlet(const std::string& uri, FunctionServlet::callback cb);

  void addGlobServlet(const std::string& uri, Servlet::ptr slt);

  void addGlobServlet(const std::string& uri, FunctionServlet::callback cb);

  void addServletCreator(const std::string& uri, IServletCreator::ptr creator);
  void addGlobServletCreator(const std::string& uri,
                             IServletCreator::ptr creator);

  template <class T>
  void addServletCreator(const std::string& uri) {
    addServletCreator(uri, std::make_shared<ServletCreator<T>>());
  }

  template <class T>
  void addGlobServletCreator(const std::string& uri) {
    addGlobServletCreator(uri, std::make_shared<ServletCreator<T>>());
  }

  void delServlet(const std::string& uri);

  void delGlobServlet(const std::string& uri);

  Servlet::ptr getDefault() const;

  void setDefault(Servlet::ptr v);

  Servlet::ptr getServlet(const std::string& uri);

  Servlet::ptr getGlobServlet(const std::string& uri);

  Servlet::ptr getMatchedServlet(const std::string& uri);

  void listAllServletCreator(
      std::map<std::string, IServletCreator::ptr>& infos);
  void listAllGlobServletCreator(
      std::map<std::string, IServletCreator::ptr>& infos);

 private:
  RWMutexType m_mutex;

  std::unordered_map<std::string, IServletCreator::ptr> m_datas;

  std::vector<std::pair<std::string, IServletCreator::ptr>> m_globs;

  Servlet::ptr m_default;
};

class NotFoundServlet : public Servlet {
 public:
  using ptr = std::shared_ptr<NotFoundServlet>;

  NotFoundServlet(const std::string& name);
  virtual int32_t handle(ddg::http::HttpRequest::ptr request,
                         ddg::http::HttpResponse::ptr response,
                         ddg::http::HttpSession::ptr session) override;

 private:
  std::string m_name;
  std::string m_content;
};

}  // namespace http
}  // namespace ddg

#endif
