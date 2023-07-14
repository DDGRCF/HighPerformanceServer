#include "ddg/http/http_server.h"

#include "ddg/iomanager.h"
#include "ddg/log.h"

static ddg::Logger::ptr g_logger = DDG_LOG_ROOT();

void run() {
  ddg::http::HttpServer::ptr server(new ddg::http::HttpServer);
  ddg::Address::ptr addr = ddg::Address::LookupAnyIPAddress("0.0.0.0:8020");
  while (!server->bind(addr)) {
    sleep(2);
  }
  server->start();
}

int main() {
  ddg::IOManager iom(4);
  iom.schedule(run);
  iom.start();
  iom.stop();
  return 0;
}
