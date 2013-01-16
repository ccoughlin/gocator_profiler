#pragma once
extern "C" {
    #include "Go2.h"
}
#include "gocatorcontrol.h"
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include <string>
#include <iostream>

class GocatorConfigurator {
public:
    static Encoder configuredEncoder(std::string& configFile);
    static Trigger* configuredTrigger(std::string& configFile);
};