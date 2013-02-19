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
#include <sstream>
#include <stdexcept>
#include <boost/filesystem.hpp>
#include <boost/thread/thread.hpp>

#define RECEIVE_TIMEOUT 100000
#define INVALID_RANGE_16BIT 0x8000

enum TravelDirection {BIDIRECTIONAL, FORWARD, BACKWARD};

typedef struct gocatorEncoder {
    std::string modelName; // Make/model of encoder
    double resolution; // Resolution of encoder (mm/"tick")
} Encoder;

typedef struct gocatorFilter {
    bool xGap, yGap, xSmooth, ySmooth;
    Go2Double xSmoothWindow, ySmoothWindow, xGapWindow, yGapWindow;
    Go2ResamplingType sampling;
} GocatorFilter;

// Controls the specified GocatorSystem.
class GocatorControl {
    public:
        GocatorControl(GocatorSystem& go2system, bool verboseFlag=false):sys(go2system), verbose(verboseFlag) {}
        void configureEncoder(Encoder& encoder);
        void configureFilter(GocatorFilter& filter);
        void targetOn();
        void targetOff();
        void recordProfile(std::string& outputFilename);
        Go2System& getSystem() {return sys.getSystem();}
        Encoder& getEncoder() {return lme;}
        void resetEncoder() {Go2System_GetEncoder(sys.getSystem(), &startingEncoderReading);}
    private:
        GocatorSystem& sys;
        bool verbose;
        Encoder lme;
        Go2Int64 startingEncoderReading;
};

// Define the various types of trigger

class Trigger {
public:
    virtual Go2Status set(GocatorControl& controller)=0;
    void setTriggerGate(bool enabled) {
        useTriggerGate = enabled;
    }
    bool isTriggerGateEnabled() {return useTriggerGate;}
    virtual std::string getTriggerType()=0;
protected:
    static const Go2TriggerSource triggerSource = GO2_TRIGGER_SOURCE_SOFTWARE;
    Go2Bool useTriggerGate;
};

// Software trigger
class SoftwareTrigger:public Trigger {
public:
    Go2Status set(GocatorControl& controller) {
        Go2System_EnableTriggerGate(controller.getSystem(), useTriggerGate);
        return Go2System_SetTriggerSource(controller.getSystem(), triggerSource);
    }
    std::string getTriggerType() {
        return std::string ("Software");
    }
protected:
    static const Go2TriggerSource triggerSource = GO2_TRIGGER_SOURCE_SOFTWARE;
};

// Digital input trigger
class InputTrigger:public Trigger {
    Go2Status set(GocatorControl& controller) {
        Go2System_EnableTriggerGate(controller.getSystem(), useTriggerGate);
        return Go2System_SetTriggerSource(controller.getSystem(), triggerSource);
    }
    std::string getTriggerType() {
        return std::string("Digital Input");;
    }
protected:
    static const Go2TriggerSource triggerSource = GO2_TRIGGER_SOURCE_INPUT;
};

// Time trigger
class TimeTrigger:public Trigger {
public:
    Go2Status set(GocatorControl& controller) {
        Go2System sys = controller.getSystem();
        double minFrameRate = Go2System_FrameRateMin(sys);
        double maxFrameRate = Go2System_FrameRateMax(sys);
        if (frameRate<minFrameRate) {
            frameRate = minFrameRate;
        } else if (frameRate>maxFrameRate) {
            frameRate = maxFrameRate;
        }
        Go2System_SetFrameRate(sys, frameRate);
        Go2System_EnableTriggerGate(controller.getSystem(), useTriggerGate);
        return Go2System_SetTriggerSource(controller.getSystem(), triggerSource);    
    }
    void setFrameRate(double framesPerSecond) {frameRate=framesPerSecond;}
    std::string getTriggerType() {
        std::ostringstream os;
        os << "Timer (" << frameRate << " cycles/s)";
        return os.str();
    }
protected:
    double frameRate; // Frame rate for triggering (Hz)
    static const Go2TriggerSource triggerSource = GO2_TRIGGER_SOURCE_TIME;
};

// Encoder Trigger
class EncoderTrigger:public Trigger {
public:
    Go2Status set(GocatorControl& controller) {
        Encoder encoder = controller.getEncoder();
        if (travel_threshold < encoder.resolution || 
            travel_threshold <= 0) {
            travel_threshold = encoder.resolution;
        }
        Go2System_SetEncoderPeriod(controller.getSystem(), travel_threshold);
        Go2System_SetEncoderTriggerMode(controller.getSystem(), travel_direction);
        Go2System_EnableTriggerGate(controller.getSystem(), useTriggerGate);
        return Go2System_SetTriggerSource(controller.getSystem(), triggerSource);
    }
    void setTravelThreshold(double threshold) {travel_threshold=threshold;}
    void setTravelDirection(TravelDirection direction) {
        switch(direction) {
        case FORWARD:
            travel_direction = GO2_ENCODER_TRIGGER_MODE_IGNORE_REVERSE;
            break;
        case BACKWARD:
            travel_direction = GO2_ENCODER_TRIGGER_MODE_TRACK_REVERSE;
            break;
        case BIDIRECTIONAL:
        default:
            travel_direction = GO2_ENCODER_TRIGGER_MODE_BIDIRECTIONAL;
        }
    }
    std::string getTriggerType() {
        std::ostringstream os;
        os << "Encoder (" << travel_threshold << " mm, ";
        switch(travel_direction) {
            case GO2_ENCODER_TRIGGER_MODE_IGNORE_REVERSE:
                os << " forward motion only)";
                break;
            case GO2_ENCODER_TRIGGER_MODE_TRACK_REVERSE:
                os << " backward motion only)";
                break;
            case GO2_ENCODER_TRIGGER_MODE_BIDIRECTIONAL:
            default:
                os << " forward/backward motion)";
        }
        return os.str();
    }
protected:
    double travel_threshold; // Desired trigger level (mm)
    Go2EncoderTriggerMode travel_direction; // Allowable trigger direction(s)
    static const Go2TriggerSource triggerSource = GO2_TRIGGER_SOURCE_ENCODER;
};
