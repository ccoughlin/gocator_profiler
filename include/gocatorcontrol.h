#pragma once
extern "C" {
    #include "Go2.h"
}
#include "go2response.h"
#include "gocatorsystem.h"

#include <fstream>
#include <ios>
#include <string>
#include <iostream>
#include <stdexcept>
#include <boost/filesystem.hpp>

#define RECEIVE_TIMEOUT 100000
#define INVALID_RANGE_16BIT 0x8000

enum TriggerDirection {BIDIRECTIONAL, FORWARD, BACKWARD};

typedef struct Encoder {
    std::string modelName; // Make/model of encoder
    double resolution; // Resolution of encoder (mm/"tick")
    double travel_threshold; // Desired trigger level (mm)
    enum TriggerDirection trigger_direction; // Allowable trigger direction(s)
} Encoder;

// Controls the specified GocatorSystem.
class GocatorControl {
    public:
        GocatorControl(GocatorSystem& go2system, bool verboseFlag=false):sys(go2system), verbose(verboseFlag) {}
        void configureEncoder(Encoder& encoder);
        void recordProfile(std::string& outputFilename);
    private:
        GocatorSystem& sys;
        bool verbose;
        Encoder lme;
};