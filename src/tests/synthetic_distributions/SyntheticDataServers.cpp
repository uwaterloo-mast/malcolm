#include <boost/log/utility/setup/console.hpp>
#include <random>
#include <stdlib.h>
#include <string>
#include <thread>
#include <unistd.h>

#include "../../core/server.h"

TaskExecutionFunc func = [](const char *data, size_t len, char *res,
                            size_t &resLen, int taskType) {
  auto start = getCurrentTime();
  memcpy(res, data, len);
  double executionTime;
  memcpy(&executionTime, data, sizeof(executionTime));
  uint64_t execTime = executionTime;
  while (getCurrentTime() - start < execTime)
    ;
  resLen = len;
};
TaskExecutionFunc funcs[] = {func};

void runServer(Server *server) { server->startRunning(); }

int main(int argc, char *argv[]) {
  Config c;
  c.parse_arguments(argc, argv);
  auto enableLog = c.get_integer("enableLog", 1);
  auto host = c.get("host");
  auto port = c.get_integer("port");
  auto id = c.get_integer("id");
  auto num = c.get_integer("num");

  boost::log::core::get()->set_logging_enabled(enableLog != 0);
  auto psink = boost::log::add_console_log();
  psink->locked_backend()->auto_flush(enableLog != 0);
  if (enableLog)
    logInit();

  Server **servers = new (std::align_val_t(64)) Server *[num];
  std::thread *serverThreads = new (std::align_val_t(64)) std::thread[num];

  for (int i = 0; i < num; i++) {
    auto adr = EndPointAddr(host, std::to_string(port + i));
    servers[i] = new Server(adr, c, funcs);
  }

  for (auto i = 0; i < num; i++) {
    for (auto j = 0; j < num; j++) {
      servers[i]->addLocalNeighbour(servers[j]->getServerID(), servers[j]);
    }
  }

  for (int i = 0; i < num; i++) {
    serverThreads[i] = std::thread(&runServer, servers[i]);
  }

  for (int i = 0; i < num; i++) {
    serverThreads[i].join();
  }
}
