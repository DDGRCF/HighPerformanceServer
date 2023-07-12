#ifndef DDG_TCP_SERVER_H_
#define DDG_TCP_SERVER_H_

#include <memory>

#include "ddg/address.h"
#include "ddg/config.h"
#include "ddg/iomanager.h"
#include "ddg/log.h"
#include "ddg/noncopyable.h"
#include "ddg/socket.h"

namespace ddg {

struct TcpServerConf {
  using ptr = std::shared_ptr<TcpServerConf>;

  std::vector<std::string> address;
  int keepalive = 0;
  int timeout = 1000 * 2 * 60;
  int ssl = 0;
  std::string id;
  /// 服务器类型，http, ws, rock
  std::string type = "http";
  std::string name;
  std::string cert_file;
  std::string key_file;
  std::string accept_worker;
  std::string io_worker;
  std::string process_worker;
  std::map<std::string, std::string> args;

  bool isValid() const;

  bool operator==(const TcpServerConf& oth) const;
};

template <>
class LexicalCast<std::string, TcpServerConf> {
 public:
  TcpServerConf operator()(const std::string& v) {
    YAML::Node node = YAML::Load(v);
    TcpServerConf conf;
    conf.id = node["id"].as<std::string>(conf.id);
    conf.type = node["type"].as<std::string>(conf.type);
    conf.keepalive = node["keepalive"].as<int>(conf.keepalive);
    conf.timeout = node["timeout"].as<int>(conf.timeout);
    conf.name = node["name"].as<std::string>(conf.name);
    conf.ssl = node["ssl"].as<int>(conf.ssl);
    conf.cert_file = node["cert_file"].as<std::string>(conf.cert_file);
    conf.key_file = node["key_file"].as<std::string>(conf.key_file);
    conf.accept_worker = node["accept_worker"].as<std::string>();
    conf.io_worker = node["io_worker"].as<std::string>();
    conf.process_worker = node["process_worker"].as<std::string>();
    conf.args = LexicalCast<std::string, std::map<std::string, std::string>>()(
        node["args"].as<std::string>(""));
    if (node["address"].IsDefined()) {
      for (size_t i = 0; i < node["address"].size(); ++i) {
        conf.address.push_back(node["address"][i].as<std::string>());
      }
    }
    return conf;
  }
};

template <>
class LexicalCast<TcpServerConf, std::string> {
 public:
  std::string operator()(const TcpServerConf& conf) {
    YAML::Node node;
    node["id"] = conf.id;
    node["type"] = conf.type;
    node["name"] = conf.name;
    node["keepalive"] = conf.keepalive;
    node["timeout"] = conf.timeout;
    node["ssl"] = conf.ssl;
    node["cert_file"] = conf.cert_file;
    node["key_file"] = conf.key_file;
    node["accept_worker"] = conf.accept_worker;
    node["io_worker"] = conf.io_worker;
    node["process_worker"] = conf.process_worker;
    node["args"] = YAML::Load(
        LexicalCast<std::map<std::string, std::string>, std::string>()(
            conf.args));
    for (auto& i : conf.address) {
      node["address"].push_back(i);
    }
    std::stringstream ss;
    ss << node;
    return ss.str();
  }
};

class TcpServer : public std::enable_shared_from_this<TcpServer>, NonCopyable {
 public:
  using ptr = std::shared_ptr<TcpServer>;

 public:
  TcpServer(ddg::IOManager* worker = ddg::IOManager::GetThis(),
            ddg::IOManager* io_worker = ddg::IOManager::GetThis(),
            ddg::IOManager* accept_workder = ddg::IOManager::GetThis());

  virtual ~TcpServer();

  virtual bool bind(ddg::Address::ptr addr, bool ssl = false);

  virtual bool bind(const std::vector<Address::ptr>& addr,
                    std::vector<Address::ptr>& fails, bool ssl = false);

  bool loadCertificates(const std::string& cert_file,
                        const std::string& key_file);

  virtual bool start();

  virtual void stop();

  uint64_t getRecvTimeout() const;

  std::string getName() const;

  void setRecvTimeout(uint64_t v);

  virtual void setName(const std::string& v);

  bool isStop() const;

  TcpServerConf::ptr getConf() const;

  void setConf(TcpServerConf::ptr v);

  void setConf(const TcpServerConf& v);

  virtual std::string toString(const std::string& prefix = "");

  std::vector<Socket::ptr> getSocks() const;

 protected:
  virtual void handleClient(Socket::ptr client);

  virtual void startAccept(Socket::ptr sock);

 protected:
  std::vector<Socket::ptr> m_socks;

  IOManager* m_worker;

  IOManager* m_ioworker;

  IOManager* m_acceptworker;

  uint64_t m_recvtimeout;

  std::string m_name;

  std::string m_type = "tcp";

  bool m_isstop;

  bool m_ssl = false;

  TcpServerConf::ptr m_conf;
};

}  // namespace ddg

#endif
