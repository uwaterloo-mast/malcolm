#ifndef DLB_IMPLEMENTATION_CONFIG_H
#define DLB_IMPLEMENTATION_CONFIG_H

#include <fstream>
#include <iostream>
#include <string>
#include <unordered_map>

#include <boost/program_options.hpp>

class Config {
public:
  Config() {}

  bool contains(std::string key) const {
    if (settings.find(key) != settings.end())
      return true;
    return false;
  }

  void set(std::string key, std::string value) { settings[key] = value; }

  std::string get(std::string key, std::string default_val = "") const {
    if (contains(key) == true)
      return settings.find(key)->second;
    return default_val;
  }

  int get_integer(std::string key, int default_val = 0) const {
    if (contains(key) == true)
      return atoi(settings.find(key)->second.c_str());
    return default_val;
  }

  float get_float(std::string key, float default_val = 0.0f) const {
    if (contains(key) == true)
      return atof(settings.find(key)->second.c_str());
    return default_val;
  }

  void load_conf_file(std::string file_name) {
    std::ifstream config_file(file_name, std::ios::binary);

    if (config_file.is_open() == false)
      std::cout << "failed to open config file\n";

    else {
      std::string key_value;
      while (std::getline(config_file, key_value)) {
        auto split_pos = key_value.find("=");
        if (split_pos != std::string::npos)
          settings[key_value.substr(0, split_pos)] =
              key_value.substr(split_pos + 1);
      }
    }
  }

  void parse_arguments(int argc, char *argv[]) {
    namespace po = boost::program_options;

    po::options_description desc("Allowed options");
    std::string config_file = "";
    desc.add_options()("config, c", po::value<std::vector<std::string>>(),
                       "key=value")(
        "config_file,cf", po::value<std::string>(&config_file), ".config file");

    // Just parse the options without storing them in the map.
    po::parsed_options parsed_options =
        po::command_line_parser(argc, argv).options(desc).run();

    po::variables_map vm;
    po::store(parsed_options, vm);
    po::notify(vm);

    auto results =
        (vm.count("config") ? vm["config"].as<std::vector<std::string>>()
                            : std::vector<std::string>());
    for (auto &res : results)
      set(res.substr(0, res.find("=")), res.substr(res.find("=") + 1));
    if (config_file != "")
      load_conf_file(config_file);
  }

  std::vector<std::string> get_vector(std::string key) const {
    auto str_val = get(key, "");
    std::stringstream ss(str_val);
    std::istream_iterator<std::string> begin(ss);
    std::istream_iterator<std::string> end;
    std::vector<std::string> res(begin, end);
    return res;
  }

  std::vector<int> get_int_vector(std::string key,
                                  std::vector<int> default_vector) const {
    std::vector<int> res;
    auto strRes = get_vector(key);
    if (strRes.size() == 0)
      return default_vector;
    for (auto &v : strRes)
      res.push_back(atoi(v.c_str()));
    return res;
  }

  std::vector<std::pair<double, double>>
  get_pair_vector(std::string key,
                  std::vector<std::pair<double, double>> default_value) const {
    auto vec = get_vector(key);
    if (vec.size() == 0)
      return default_value;
    std::vector<std::pair<double, double>> res;
    for (auto &v : vec)
      res.emplace_back(atof(v.substr(0, v.find(":")).c_str()),
                       atof(v.substr(v.find(":") + 1).c_str()));
    return res;
  }

  friend std::ostream &operator<<(std::ostream &os, const Config &c) {
    for (auto &key_value : c.settings)
      os << key_value.first << "=" << key_value.second << "\n";
    return os;
  }

private:
  std::unordered_map<std::string, std::string> settings;
};

#endif // DLB_IMPLEMENTATION_CONFIG_H
