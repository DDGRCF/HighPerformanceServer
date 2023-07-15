#include "ddg/log.h"
#include "ddg/uri.h"

static ddg::Logger::ptr g_logger = DDG_LOG_ROOT();

int main() {
  ddg::Uri::ptr uri = ddg::Uri::Create(
      "http://admin@www.baidu.com/test/中文/"
      "uri?day=100&name=rcf&age=18#frag中文");
  DDG_LOG_DEBUG(g_logger) << *uri;
  DDG_LOG_DEBUG(g_logger) << uri->getFragment();
  DDG_LOG_DEBUG(g_logger) << uri->getHost();
  DDG_LOG_DEBUG(g_logger) << uri->getQuery();

  auto addr = uri->createAddress();
  DDG_LOG_DEBUG(g_logger) << *addr;

  return 0;
}
