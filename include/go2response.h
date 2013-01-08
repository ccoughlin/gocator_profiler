#pragma once
extern "C" {
    #include "Go2.h"
}
#include <string>

std::string getResponseString(std::string function, Go2Status returnCode);