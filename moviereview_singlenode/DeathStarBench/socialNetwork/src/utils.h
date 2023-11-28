#ifndef SOCIAL_NETWORK_MICROSERVICES_UTILS_H
#define SOCIAL_NETWORK_MICROSERVICES_UTILS_H

#include <string>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

#include "logger.h"

namespace social_network{
using json = nlohmann::json;

int load_config_file(const std::string &file_name, json *config_json) {
  std::ifstream json_file;
  json_file.open(file_name);
  if (json_file.is_open()) {
    json_file >> *config_json;
    json_file.close();
    return 0;
  }
  else {
    LOG(error) << "Cannot open service-config.json";
    return -1;
  }
};

int load_config(json *config_json) {
  char* config_json_str = getenv("CONFIG_JSON_STR");
  if (config_json_str != nullptr && strlen(config_json_str) > 0) {
    *config_json = json::parse(config_json_str);
    return 0;
  } else {
    return load_config_file("config/service-config.json", config_json);
  }
}

} //namespace social_network

#endif //SOCIAL_NETWORK_MICROSERVICES_UTILS_H
