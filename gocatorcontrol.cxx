#include "gocatorcontrol.h"
namespace filesystem = boost::filesystem;

// Configures Gocator 20x0 to use an attached encoder.
void GocatorControl::configureEncoder(Encoder& encoder) {
    lme = encoder;
    std::string SetTriggerResponse = getResponseString("Go2System_SetTriggerSource", 
                                           Go2System_SetTriggerSource(sys.getSystem(), GO2_TRIGGER_SOURCE_ENCODER));
    if (verbose) {
        std::cout << SetTriggerResponse << std::endl;
    }
 
    std::string SetResolutionResponse = getResponseString("Go2System_SetTravelResolution", 
                                              Go2System_SetTravelResolution(sys.getSystem(), lme.resolution));
    if (verbose) {
        std::cout << SetResolutionResponse << std::endl;
    }

    std::string DisableTriggerGateResponse = getResponseString("Go2System_EnableTriggerGate", 
                                                   Go2System_EnableTriggerGate(sys.getSystem(), GO2_FALSE));
    if (verbose) {
        std::cout << DisableTriggerGateResponse << std::endl;
    }

    std::string SetEncoderPeriodResponse = getResponseString("Go2System_SetEncoderPeriod", 
                                                 Go2System_SetEncoderPeriod(sys.getSystem(), lme.travel_threshold));
    if (verbose) {
        std::cout << SetEncoderPeriodResponse << std::endl;
    }

    Go2EncoderTriggerMode trigger_mode;
    switch(encoder.trigger_direction) {
        case FORWARD:
            trigger_mode = GO2_ENCODER_TRIGGER_MODE_IGNORE_REVERSE;
            break;
        case BACKWARD:
            trigger_mode = GO2_ENCODER_TRIGGER_MODE_TRACK_REVERSE;
            break;
        default:
            trigger_mode = GO2_ENCODER_TRIGGER_MODE_BIDIRECTIONAL;
    }
    std::string SetTriggerModeResponse = getResponseString("Go2System_SetEncoderTriggerMode", 
                                               Go2System_SetEncoderTriggerMode(sys.getSystem(), trigger_mode));
    if (verbose) {
        std::cout << SetTriggerModeResponse << std::endl;
    }
}
    
// Records range profiles to disk as comma-delimited ASCII.
void GocatorControl::recordProfile(std::string& outputFilename) {
    try {
        filesystem::remove(outputFilename.c_str());
    } catch (filesystem::filesystem_error &err) {
        std::cerr << "<< Unable to overwrite '" << outputFilename << ",' appending >>" << std::endl;
    }
    std::ofstream fidout;
    fidout.open(outputFilename.c_str(), std::ios_base::app);
    if (fidout.is_open()) {
        fidout << "# File format: X Position [mm], Y Position [mm], Z Range [mm]" << std::endl;
        Go2ProfileData data = GO2_NULL;
        Go2Data dataItem;
        Go2Int64 encoderCounter;
        unsigned int itemCount = 0;
        std::string StartResponse = getResponseString("Go2System_Start",Go2System_Start(sys.getSystem()));
        if (verbose) {
            std::cout << StartResponse << std::endl;
        }
        Go2Status returnCode = Go2System_ConnectData(sys.getSystem(), GO2_NULL, GO2_NULL);
        if (verbose) {
            std::cout << getResponseString("Go2System_ConnectData", returnCode) << std::endl;
        }
        if (returnCode != GO2_OK) {
            std::cerr << "\n<< Initialization failed, aborting >>" << std::endl;
            throw std::runtime_error("Gocator 20x0 initialization failed");
        }
        while(true) {
            Go2Status returnCode = Go2System_ReceiveData(sys.getSystem(), RECEIVE_TIMEOUT, &data);
            if (returnCode == GO2_OK) {
                itemCount = Go2Data_ItemCount(data);
                for (unsigned int j=0; j<itemCount; j++) {
                    dataItem = Go2Data_ItemAt(data, j);
                    short* profileData = Go2ProfileData_Ranges(dataItem);
                    unsigned int profilePointCount = Go2ProfileData_Width(dataItem);
                    double XResolution = Go2ProfileData_XResolution(dataItem);
                    double ZResolution = Go2ProfileData_ZResolution(dataItem);
                    double XOffset = Go2ProfileData_XOffset(dataItem);
                    double ZOffset = Go2ProfileData_ZOffset(dataItem);
                    // number of ticks of encoder
                    std::string GetEncoderResponse = getResponseString("Go2System_GetEncoder", Go2System_GetEncoder(sys.getSystem(), &encoderCounter));
                    if (verbose) {
                        std::cout << GetEncoderResponse << std::endl; 
                    }
                    for(unsigned int arrayIndex=0;arrayIndex<profilePointCount; ++arrayIndex) {
                        if (profileData[arrayIndex] != INVALID_RANGE_16BIT) {
                            fidout << XOffset+XResolution*arrayIndex << "," << encoderCounter*lme.resolution << "," << ZOffset+ZResolution*profileData[arrayIndex] << std::endl << std::flush;
                        } else {
                            if (verbose) {
                                std::cout << "Invalid reading, skipped." << std::endl;
                            }
                        }
                    }
                }
            }
        }
        fidout.flush();
        fidout.close();
        if (fidout.fail()) {
            std::cerr << "<< Encountered error writing to '" << outputFilename << ",' data may have been lost.\n" << std::endl;
        }
    } else {
        std::cerr << "<< Unable to open/write to output file '" << outputFilename << "', aborting >>" << std::endl;
        throw std::runtime_error("Unable to write to output");
    }
}