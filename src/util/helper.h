#ifndef DLB_HELPER_H
#define DLB_HELPER_H

#include <iostream>
#include <pthread.h>

#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sinks/text_file_backend.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/utility/setup/file.hpp>

inline void setCoreAffinity(int coreNumber) {
  cpu_set_t cpuSet;
  CPU_ZERO(&cpuSet);
  CPU_SET(coreNumber, &cpuSet);
  int rc = pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuSet);
  if (rc != 0)
    std::cout << "Error calling pthread_setaffinity_np: " << rc << std::endl;
}

static uint64_t getCurrentTime() {
  struct timespec ts;
  clock_gettime(CLOCK_REALTIME, &ts);
  uint64_t t = ts.tv_sec * 1000 * 1000 * 1000 + ts.tv_nsec;
  return t;
}

inline void logInit() {
  boost::log::register_simple_formatter_factory<
      boost::log::trivial::severity_level, char>("Severity");

  boost::log::add_file_log(
      boost::log::keywords::target = "logs/",
      boost::log::keywords::file_name = "%y%m%d_%3N.log",
      boost::log::keywords::rotation_size = 10 * 1024 * 1024,
      boost::log::keywords::scan_method =
          boost::log::sinks::file::scan_matching,
      boost::log::keywords::format = "[%TimeStamp%][%Severity%]: %Message%");

  boost::log::core::get()->set_filter(boost::log::trivial::severity >=
                                      boost::log::trivial::info);
}

#endif // DLB_HELPER_H
