#include "ddg/http/http.h"

#include <iostream>

#include "ddg/log.h"

void test_request() {
  ddg::http::HttpRequest::ptr req = std::make_shared<ddg::http::HttpRequest>();
  req->setHeader("host", "www.ddg.top");
  req->setBody("hello ddg");
  req->dump(std::cout) << std::endl;
}

void test_response() {
  ddg::http::HttpResponse::ptr rsp =
      std::make_shared<ddg::http::HttpResponse>();
  rsp->setHeader("X-X", "ddg");
  rsp->setBody("hello ddg");
  rsp->setStatus((ddg::http::HttpStatus)400);
  rsp->setClose(false);

  rsp->dump(std::cout) << std::endl;
}

int main(int argc, char** argv) {
  test_request();
  std::cout << std::string(150, '*') << std::endl;
  test_response();
  return 0;
}
