#pragma once

#include "logging.hh"

namespace nix {

struct LogFormat : std::string {
    using std::string::string;

    LogFormat(std::string s) : std::string(s) { }
    LogFormat(char* s) : std::string(s) { }
};

struct LoggerBuilder {
  std::string name;
  std::function<Logger*()> builder;
};

std::vector<std::shared_ptr<LoggerBuilder>> getRegisteredLoggers();

void registerLogger(std::string name, std::function<Logger*()> builder);

}


