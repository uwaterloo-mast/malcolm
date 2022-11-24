#include "../../core/driver.h"
#include "../../util/config.h"
#include "../../util/distributions.h"

void runDriver(Driver *driver) { driver->startRunning(); }

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

  auto seed = getCurrentTime();
  Distribution *distribution;
  if (c.get("executionTimeDistribution", "Exp") == "Exp")
    distribution =
        new Exponential(seed, c.get_integer("meanExecutionTime", 50000));
  else if (c.get("executionTimeDistribution", "Exp") == "MultiModal")
    distribution =
        new DetMultiModal(seed, c.get_pair_vector("meanExecutionTime",
                                                  {{9, 50000}, {1, 500000}}));
  else if (c.get("executionTimeDistribution", "Exp") == "HYPEXP")
    distribution =
        new ExpMultiModal(seed, c.get_pair_vector("meanExecutionTime",
                                                  {{9, 50000}, {1, 500000}}));
  else
    throw "Execution time distribution undefined!";

  TaskGenerationFunc func = [distribution](char *data, size_t &len,
                                           uint8_t &type) {
    auto serviceTime = distribution->getNext();
    memcpy(data, &(serviceTime.second), sizeof(serviceTime.second));
    len = sizeof(serviceTime.second);
    type = serviceTime.first;
  };

  Driver **drivers = new (std::align_val_t(64)) Driver *[num];
  std::thread *driverThreads = new (std::align_val_t(64)) std::thread[num];

  for (int i = 0; i < num; i++) {
    auto adr = EndPointAddr(host, std::to_string(port + i));
    drivers[i] = new Driver(adr, c, func);
  }

  for (int i = 0; i < num; i++) {
    driverThreads[i] = std::thread(&runDriver, drivers[i]);
  }

  for (int i = 0; i < num; i++) {
    driverThreads[i].join();
  }
}
