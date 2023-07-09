#include "ddg/address.h"

#include <arpa/inet.h>
#include <ifaddrs.h>
#include <netdb.h>
#include <sys/types.h>

#include "ddg/endian.h"
#include "ddg/log.h"

namespace ddg {

static Logger::ptr g_logger = DDG_LOG_ROOT();

/**
 * @brief 将高位置为1，剩余位数为0
 * @bits 位数
 **/
template <class T>
static T CreateMask(uint32_t bits) {
  return (1 << (sizeof(T) * 8 - bits)) - 1;
}

template <class T>
static uint32_t CountBytes(T x) {
  int count = 0;
  while (x) {
    count++;
    x &= (x - 1);
  }
  return count;
}

bool Address::Lookup(std::vector<Address::ptr>& result, const std::string& host,
                     int family, int type, int protocol) {
  addrinfo hints, *results, *next;
  hints.ai_flags = 0;
  hints.ai_family = family;
  hints.ai_socktype =
      type;  // sock_stream 将限制每个地址(三次是连接，数据报，原始嵌套字)
  hints.ai_protocol = protocol;
  hints.ai_addrlen = 0;
  hints.ai_canonname = nullptr;
  hints.ai_addr = nullptr;
  hints.ai_next = nullptr;

  std::string node;
  const char* service = nullptr;  // 端口号或者是(ftp,http等名字)

  if (!host.empty() && host[0] == '[') {
    const char* endipv6 = static_cast<const char*>(
        memchr(host.c_str() + 1, ']', host.size() - 1));
    if (endipv6) {
      if (*(endipv6 + 1) == ':') {
        service = endipv6 + 2;
      }
    }
    node = host.substr(1, endipv6 - host.c_str() - 1);
  }

  if (node.empty()) {
    service = static_cast<const char*>(memchr(host.c_str(), ':', host.size()));
    if (service) {
      if (!memchr(service + 1, ':', host.c_str() + host.size() - service - 1)) {
        node = host.substr(0, service - host.c_str());
        ++service;
      }
    }
  }

  if (node.empty()) {
    node = host;
  }

  int error = getaddrinfo(node.c_str(), service, &hints, &results);
  if (error) {
    DDG_LOG_DEBUG(g_logger)
        << "Address::Lookup getaddress(" << host << ", " << family << ", "
        << type << ") err=" << error << " errstr=" << gai_strerror(error);
    return false;
  }

  next = results;
  while (next) {
    result.push_back(
        Create(next->ai_addr, static_cast<socklen_t>(next->ai_addrlen)));
    next = next->ai_next;
  }

  freeaddrinfo(results);
  return !result.empty();
}

Address::ptr Address::LookupAny(const std::string& host, int family, int type,
                                int protocol) {
  std::vector<Address::ptr> result;
  if (Lookup(result, host, family, type, protocol)) {
    return result[0];
  }
  return nullptr;
}

IPAddress::ptr Address::LookupAnyIPAddress(const std::string& host, int family,
                                           int type, int protocol) {
  std::vector<Address::ptr> result;
  if (Lookup(result, host, family, type, protocol)) {
    for (auto& i : result) {
      IPAddress::ptr v = std::dynamic_pointer_cast<IPAddress>(i);
      if (v) {
        return v;
      }
    }
  }
  return nullptr;
}

bool Address::GetInterfaceAddresses(
    std::multimap<std::string, std::pair<Address::ptr, uint32_t>>& result,
    int family) {
  struct ifaddrs *next, *results;
  if (getifaddrs(&results) != 0) {
    DDG_LOG_DEBUG(g_logger)
        << "Address::GetInterfaceAddresses getifaddrs "
        << " err = " << errno << " errstr = " << strerror(errno);
    return false;
  }

  try {
    for (next = results; next; next = next->ifa_next) {
      Address::ptr addr;
      uint32_t prefix_len = ~0u;
      if (family != AF_UNSPEC && family != next->ifa_addr->sa_family) {
        continue;
      }
      switch (next->ifa_addr->sa_family) {
        case AF_INET: {
          addr = Create(next->ifa_addr, sizeof(sockaddr_in));
          uint32_t netmask = reinterpret_cast<sockaddr_in*>(next->ifa_netmask)
                                 ->sin_addr.s_addr;
          prefix_len = CountBytes(netmask);
          break;
        }
        case AF_INET6: {
          addr = Create(next->ifa_addr, sizeof(sockaddr_in6));
          in6_addr& netmask =
              reinterpret_cast<sockaddr_in6*>(next->ifa_netmask)->sin6_addr;
          prefix_len = 0;
          for (int i = 0; i < 16; i++) {
            prefix_len += CountBytes(netmask.s6_addr[i]);
          }
          break;
        }
        default:
          addr = Create(next->ifa_addr, sizeof(sockaddr));
          break;
      }

      if (addr) {
        result.insert(
            std::make_pair(next->ifa_name, std::make_pair(addr, prefix_len)));
      }
    }
  } catch (...) {
    DDG_LOG_ERROR(g_logger) << "Address::GetInterfaceAddresses exception";
    freeifaddrs(results);
    return false;
  }
  freeifaddrs(results);
  return !result.empty();
}

bool Address::GetInterfaceAddresses(
    std::vector<std::pair<Address::ptr, uint32_t>>& result,
    const std::string& iface, int family) {
  if (iface.empty() || iface == "*") {
    if (family == AF_INET || family == AF_UNSPEC) {
      result.push_back({std::make_shared<IPv4Address>(), 0u});
    }
    if (family == AF_INET6 || family == AF_UNSPEC) {
      result.push_back({std::make_shared<IPv6Address>(), 0u});
    }
    return true;
  }

  std::multimap<std::string, std::pair<Address::ptr, uint32_t>> results;
  if (!GetInterfaceAddresses(results, family)) {
    return false;
  }

  auto its = results.equal_range(iface);
  for (; its.first != its.second; its.first++) {
    result.push_back(its.first->second);
  }
  return !results.empty();
}

Address::ptr Address::Create(const sockaddr* addr, socklen_t addrlen) {
  if (addr == nullptr) {
    return nullptr;
  }
  Address::ptr result;
  switch (addr->sa_family) {
    case AF_INET:
      result.reset(
          new IPv4Address(*reinterpret_cast<const sockaddr_in*>(addr)));
      break;
    case AF_INET6:
      result.reset(
          new IPv6Address(*reinterpret_cast<const sockaddr_in6*>(addr)));
      break;
    default:
      result.reset(new UnknownAddress(*addr));
      break;
  }
  return result;
}

IPAddress::ptr IPAddress::Create(const char* address, uint16_t port) {
  addrinfo hints, *results;
  bzero(&hints, sizeof(hints));

  hints.ai_flags = AI_NUMERICHOST;
  hints.ai_family = AF_UNSPEC;

  int error = getaddrinfo(address, nullptr, &hints, &results);
  if (error) {
    DDG_LOG_DEBUG(g_logger) << "IPAddress::Create(" << address << ", " << port
                            << ") error = " << error << " errno = " << errno
                            << " errstr = " << strerror(errno);
    return nullptr;
  }

  try {
    IPAddress::ptr result =
        std::dynamic_pointer_cast<IPAddress>(Address::Create(
            results->ai_addr, static_cast<socklen_t>(results->ai_addrlen)));
    if (result) {
      result->setPort(port);
    }

    freeaddrinfo(results);
    return result;
  } catch (...) {
    freeaddrinfo(results);
    return nullptr;
  }
}

int Address::getFamily() const {
  return getAddr()->sa_family;
}

std::string Address::toString() const {
  std::stringstream ss;
  insert(ss);
  return ss.str();
}

std::ostream& operator<<(std::ostream& os, const Address& rhs) {
  return rhs.insert(os);
}

bool Address::operator<(const Address& rhs) const {
  socklen_t minlen = std::min(getAddrLen(), rhs.getAddrLen());
  int result = memcmp(getAddr(), rhs.getAddr(), minlen);
  if (result < 0) {
    return true;
  } else if (result > 0) {
    return false;
  } else if (getAddrLen() < rhs.getAddrLen()) {
    return true;
  }
  return false;
}

bool Address::operator==(const Address& rhs) const {
  return getAddrLen() == rhs.getAddrLen() &&
         memcmp(getAddr(), rhs.getAddr(), getAddrLen()) == 0;
}

bool Address::operator!=(const Address& rhs) const {
  return !(*this == rhs);
}

IPv4Address::ptr IPv4Address::Create(const char* address, uint16_t port) {
  IPv4Address::ptr ret = std::make_shared<IPv4Address>();
  ret->m_addr.sin_port = byteswapOnLittleEndian(port);
  int result = inet_pton(AF_INET, address, &ret->m_addr.sin_addr);
  if (result <= 0) {
    DDG_LOG_DEBUG(g_logger) << "IPv4Address::Create(" << address << ", " << port
                            << ") ret = " << result << " errno = " << errno
                            << " errstr = " << strerror(errno);
    return nullptr;
  }
  return ret;
}

IPv4Address::IPv4Address(const sockaddr_in& address) {
  m_addr = address;
}

IPv4Address::IPv4Address(uint32_t address,
                         uint16_t port) {  // 32位swap后赋给16位会有风险
  bzero(&m_addr, sizeof(m_addr));
  m_addr.sin_family = AF_INET;
  m_addr.sin_port = AF_INET;
  m_addr.sin_port = byteswapOnLittleEndian(port);
  m_addr.sin_addr.s_addr = byteswapOnLittleEndian(address);
}

const sockaddr* IPv4Address::getAddr() const {
  return reinterpret_cast<const sockaddr*>(&m_addr);
}

socklen_t IPv4Address::getAddrLen() const {
  return sizeof(m_addr);
}

// 将127.0.0.1转化为主机字节序就是将每一位进行转化
std::ostream& IPv4Address::insert(std::ostream& os) const {
  os << "[IPv4Address] ";
  uint32_t addr = byteswapOnLittleEndian(m_addr.sin_addr.s_addr);

  os << ((addr >> 24) & 0xff) << "." << ((addr >> 16) & 0xff) << "."
     << ((addr >> 8) & 0xff) << "." << (addr & 0xff);
  os << ":" << byteswapOnLittleEndian(m_addr.sin_port);

  return os;
}

IPAddress::ptr IPv4Address::broadcastAddress(uint32_t prefix_len) {
  if (prefix_len > 32) {
    return nullptr;
  }

  sockaddr_in baddr(m_addr);
  baddr.sin_addr.s_addr |=
      byteswapOnLittleEndian(CreateMask<uint32_t>(prefix_len));
  return std::make_shared<IPv4Address>(baddr);
}

IPAddress::ptr IPv4Address::networdAddress(uint32_t prefix_len) {
  if (prefix_len > 32) {
    return nullptr;
  }

  sockaddr_in baddr(m_addr);
  baddr.sin_addr.s_addr &=
      byteswapOnLittleEndian(CreateMask<uint32_t>(prefix_len));
  return std::make_shared<IPv4Address>(baddr);
}

IPAddress::ptr IPv4Address::subnetMask(uint32_t prefix_len) {
  sockaddr_in subnet;
  bzero(&subnet, sizeof(subnet));
  subnet.sin_family = AF_INET;
  subnet.sin_addr.s_addr =
      ~byteswapOnLittleEndian(CreateMask<uint32_t>(prefix_len));
  return std::make_shared<IPv4Address>(subnet);
}

uint32_t IPv4Address::getPort() const {
  return byteswapOnLittleEndian(m_addr.sin_port);
}

void IPv4Address::setPort(uint16_t v) {
  m_addr.sin_port = byteswapOnLittleEndian(v);
}

IPv6Address::ptr IPv6Address::Create(const char* address, uint16_t port) {
  IPv6Address::ptr ret = std::make_shared<IPv6Address>();
  ret->m_addr.sin6_port = byteswapOnLittleEndian(port);
  int result = inet_pton(AF_INET6, address, &ret->m_addr.sin6_addr);
  if (result <= 0) {
    DDG_LOG_DEBUG(g_logger) << "IPv6Address::Create(" << address << ", " << port
                            << ") rt = " << result << " errno = " << errno
                            << " errstr = " << strerror(errno);
    return nullptr;
  }
  return ret;
}

IPv6Address::IPv6Address() {
  bzero(&m_addr, sizeof(m_addr));
  m_addr.sin6_family = AF_INET6;
}

IPv6Address::IPv6Address(const sockaddr_in6& address) {
  m_addr = address;
}

IPv6Address::IPv6Address(const uint8_t address[16], uint16_t port) {
  bzero(&m_addr, sizeof(m_addr));
  m_addr.sin6_family = AF_INET6;
  m_addr.sin6_port = byteswapOnLittleEndian(port);
  memcpy(&m_addr.sin6_addr.s6_addr, address, 16);
}

const sockaddr* IPv6Address::getAddr() const {
  return reinterpret_cast<const sockaddr*>(&m_addr);
}

socklen_t IPv6Address::getAddrLen() const {
  return sizeof(m_addr);
}

// TODO:
std::ostream& IPv6Address::insert(std::ostream& os) const {
  os << "[IPv6Address] [";
  const uint16_t* addr =
      reinterpret_cast<const uint16_t*>(m_addr.sin6_addr.s6_addr);
  bool used_zeros = false;
  for (size_t i = 0; i < 8; ++i) {
    if (addr[i] == 0 && !used_zeros) {
      continue;
    }
    if (i && addr[i - 1] == 0 && !used_zeros) {
      os << ":";
      used_zeros = true;
    }
    if (i) {
      os << ":";
    }
    os << std::hex << (int)byteswapOnLittleEndian(addr[i]) << std::dec;
  }

  if (!used_zeros && addr[7] == 0) {
    os << "::";
  }

  os << "]:" << byteswapOnLittleEndian(m_addr.sin6_port);
  return os;
}

// TODO:
IPAddress::ptr IPv6Address::broadcastAddress(uint32_t prefix_len) {
  sockaddr_in6 baddr(m_addr);
  baddr.sin6_addr.s6_addr[prefix_len / 8] |=
      CreateMask<uint8_t>(prefix_len % 8);
  for (int i = prefix_len / 8 + 1; i < 16; i++) {
    baddr.sin6_addr.s6_addr[i] = 0xff;
  }
  return std::make_shared<IPv6Address>(baddr);
}

IPAddress::ptr IPv6Address::networdAddress(uint32_t prefix_len) {
  sockaddr_in6 baddr(m_addr);
  baddr.sin6_addr.s6_addr[prefix_len / 8] &=
      CreateMask<uint8_t>(prefix_len % 8);
  for (int i = prefix_len / 8 + 1; i < 16; i++) {
    baddr.sin6_addr.s6_addr[i] = 0x00;
  }
  return IPv6Address::ptr(new IPv6Address(baddr));
}

IPAddress::ptr IPv6Address::subnetMask(uint32_t prefix_len) {
  sockaddr_in6 subnet;
  memset(&subnet, 0, sizeof(subnet));
  subnet.sin6_family = AF_INET6;
  subnet.sin6_addr.s6_addr[prefix_len / 8] =
      ~CreateMask<uint8_t>(prefix_len % 8);

  for (uint32_t i = 0; i < prefix_len / 8; ++i) {
    subnet.sin6_addr.s6_addr[i] = 0xff;
  }
  return IPv6Address::ptr(new IPv6Address(subnet));
}

uint32_t IPv6Address::getPort() const {
  return byteswapOnLittleEndian(m_addr.sin6_port);
}

void IPv6Address::setPort(uint16_t v) {
  m_addr.sin6_port = byteswapOnLittleEndian(v);
}

UnixAddress::UnixAddress() {
  bzero(&m_addr, sizeof(m_addr));
  m_addr.sun_family = AF_UNIX;
  m_length = offsetof(sockaddr_un, sun_path) + sizeof(m_addr.sun_path);
}

UnixAddress::UnixAddress(const std::string& path) {
  if (path.empty()) {
    return;
  }
  bzero(&m_addr, sizeof(m_addr));
  m_addr.sun_family = AF_UNIX;

  if (m_length > sizeof(m_addr.sun_path)) {
    throw std::logic_error("path too long");
  }

  if (!path.empty()) {
    memcpy(m_addr.sun_path, path.c_str(), path.size() + 1);
    m_length = offsetof(sockaddr_un, sun_path) + path.size() + 1;
  } else {
    m_length = offsetof(sockaddr_un, sun_path);
  }
}

void UnixAddress::setAddrLen(uint32_t v) {
  m_length = v;
}

const sockaddr* UnixAddress::getAddr() const {
  return reinterpret_cast<const sockaddr*>(&m_addr);
}

socklen_t UnixAddress::getAddrLen() const {
  return m_length;
}

// TODO: 会出现'\0'的情况吗
std::string UnixAddress::getPath() const {
  return m_addr.sun_path;
}

std::ostream& UnixAddress::insert(std::ostream& os) const {
  os << "[UnixAddress] " << m_addr.sun_path;
  return os;
}

UnknownAddress::UnknownAddress(int family) {
  bzero(&m_addr, sizeof(m_addr));
  m_addr.sa_family = family;
}

UnknownAddress::UnknownAddress(const sockaddr& addr) {
  m_addr = addr;
}

const sockaddr* UnknownAddress::getAddr() const {
  return reinterpret_cast<const sockaddr*>(&m_addr);
}

socklen_t UnknownAddress::getAddrLen() const {
  return sizeof(m_addr);
}

std::ostream& UnknownAddress::insert(std::ostream& os) const {
  os << "[UnknownAddress] family = " << m_addr.sa_family;
  return os;
}

}  // namespace ddg
