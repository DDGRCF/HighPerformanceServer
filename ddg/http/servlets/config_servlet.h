#ifndef DDG_HTTP_SERVLETS_CONFIG_SERVLET_H_
#define DDG_HTTP_SERVLETS_CONFIG_SERVLET_H_

#include "ddg/http/servlet.h"

namespace ddg {

namespace http {

namespace servlet {

class ConfigServlet : public Servlet {
 public:
  ConfigServlet();

  virtual int32_t handle(ddg::http::HttpRequest::ptr request,
                         ddg::http::HttpResponse::ptr response,
                         ddg::http::HttpSession::ptr session) override;
};

}  // namespace servlet

}  // namespace http

}  // namespace ddg

#endif
