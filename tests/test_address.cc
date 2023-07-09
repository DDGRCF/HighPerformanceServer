#include <iostream>
#include <string>

#include "ddg/address.h"
#include "ddg/log.h"

static ddg::Logger::ptr g_logger = DDG_LOG_ROOT();

void test_look_up() {
  std::vector<ddg::Address::ptr> addrs;
  bool v = ddg::Address::Lookup(addrs, "www.github.com") &&
           ddg::Address::Lookup(addrs, "www.baidu.com") &&
           ddg::Address::Lookup(addrs, "localhost:8080");

  if (!v) {
    DDG_LOG_ERROR(g_logger) << "lookup fail!";
    return;
  }

  for (size_t i = 0; i < addrs.size(); i++) {
    DDG_LOG_DEBUG(g_logger) << i << " - " << addrs[i]->toString();
  }
}

void test_iface() {
  std::multimap<std::string, std::pair<ddg::Address::ptr, uint32_t>> results;

  bool v = ddg::Address::GetInterfaceAddresses(results, AF_UNSPEC);
  if (!v) {
    DDG_LOG_DEBUG(g_logger) << "GetInterfaceAddresses fail!";
    return;
  }

  for (auto& elem : results) {
    DDG_LOG_DEBUG(g_logger)
        << elem.first << " - " << elem.second.first->toString() << " - "
        << elem.second.second;
  }
}

void test_ip() {
  ddg::IPv4Address::ptr addr_ipv4 = ddg::IPv4Address::Create("127.0.0.1");
  ddg::IPv6Address::ptr addr_ipv6 =
      ddg::IPv6Address::Create("fe80::b53e:a092:ba76:d44d");
  if (addr_ipv4) {
    DDG_LOG_DEBUG(g_logger) << *addr_ipv4;
  }
  if (addr_ipv6) {
    DDG_LOG_DEBUG(g_logger) << *addr_ipv6;
  }
}

int main() {
  std::cout << std::string(150, '*') << std::endl;
  test_ip();
  std::cout << std::string(150, '*') << std::endl;
  test_look_up();
  std::cout << std::string(150, '*') << std::endl;
  test_iface();
  return 0;
}
