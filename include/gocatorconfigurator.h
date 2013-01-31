#pragma once
extern "C" {
    #include "Go2.h"
}
#include "gocatorcontrol.h"
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include <string>
#include <iostream>

typedef struct gocatorAddress {
    Go2AddressInfo addr;
    bool reconfigure;
} GocatorAddress;

class GocatorConfigurator {
public:
    static Go2UInt32 deviceID(std::string& configFile);
    static Encoder configuredEncoder(std::string& configFile);
    static Trigger* configuredTrigger(std::string& configFile);
    static GocatorAddress configuredNetworkConnection(std::string& configFile);
};