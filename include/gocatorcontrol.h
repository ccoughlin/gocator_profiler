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

#define RECEIVE_TIMEOUT 100000
#define INVALID_RANGE_16BIT 0x8000

// Controls the specified GocatorSystem.
class GocatorControl {
    public:
        GocatorControl(GocatorSystem& go2system, bool verboseFlag=false):sys(go2system), verbose(verboseFlag) {}
        void configureEncoder(double encoderResolution, double triggerThreshold);
        void recordProfile(std::string& outputFilename);
    private:
        GocatorSystem& sys;
        bool verbose;
        double resolution;  // Encoder resolution in mm (or mm/ticks if you prefer)
        double travel_threshold; // Trigger threshold - # of ticks to cause a trigger
};