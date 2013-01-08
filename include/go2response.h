#pragma once
extern "C" {
    #include "Go2.h"
}
#include <string>
// Simple Gocator to English translator
std::string getResponseString(std::string function, Go2Status returnCode);