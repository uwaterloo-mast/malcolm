#ifndef DLB_EndPointAddr_H
#define DLB_EndPointAddr_H

#include <functional>
#include <iostream>
#include <string>

struct EndPointAddrNative {
  unsigned port;
  uint8_t host[4] = {0, 0, 0, 0};
};

class EndPointAddr {
public:
  EndPointAddr() {}
  EndPointAddr(const std::string &URI)
      : host(URI.substr(0, URI.find(":"))),
        port(URI.substr(URI.find(":") + 1)) {}
  EndPointAddr(const std::string &host, const std::string &port)
      : host(host), port(port) {}
  EndPointAddr(const EndPointAddrNative &nativeAddr) {
    host = "";
    bool localhost = true;
    for (int i = 0; i < 4; i++) {
      if ((int)nativeAddr.host[i] != 0)
        localhost = false;
      host = host + std::to_string((int)nativeAddr.host[i]) + ".";
    }
    host.pop_back();
    if (localhost)
      host = "localhost";
    port = std::to_string(nativeAddr.port);
  }
  EndPointAddr(const EndPointAddr &addr) {
    host = addr.host;
    port = addr.port;
  }
  const std::string getURI() const { return host + ":" + port; }

  int getPortNumber() const { return atoi(port.c_str()); }

  std::string getHostName() const { return host; }

  static void split_ip(std::string s, std::string delimiter,
                       uint8_t *splited_s) {
    size_t pos_start = 0, pos_end, delim_len = delimiter.length();
    std::string token;
    auto res = splited_s;

    while ((pos_end = s.find(delimiter, pos_start)) != std::string::npos) {
      token = s.substr(pos_start, pos_end - pos_start);
      pos_start = pos_end + delim_len;
      *(res++) = std::stoi(token.c_str());
    }

    *res = std::stoi(s.substr(pos_start));
  }

  void toNative(EndPointAddrNative &adr) const {
    adr.port = getPortNumber();
    getHostNameInt(adr.host);
  }

  void getHostNameInt(uint8_t *ip) const {
    if (host != "localhost") {
      EndPointAddr::split_ip(host, ".", ip);
    } else
      for (int i = 0; i < 4; i++)
        (*ip++) = 0;
  }

  bool operator==(const EndPointAddr &adr) const {
    return adr.host == host && adr.port == port;
  }

  struct HashFunction {
  public:
    size_t operator()(const EndPointAddr &env_addr) const {
      return std::hash<std::string>()(env_addr.getURI());
    }
  };

private:
  std::string host;
  std::string port;
};

#endif // DLB_EndPointAddr_H
