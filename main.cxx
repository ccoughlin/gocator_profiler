extern "C" {
    #include "Go2.h"
}
#include "go2response.h"

#include <iostream>
#include <fstream>
#include <ios>
#include <string>

#define RECEIVE_TIMEOUT 100000
#define INVALID_RANGE_16BIT 0x8000

Go2System initGo2() {
    Go2System go2system;
    Go2User user = GO2_USER_ADMIN;
    Go2Char* password = new Go2Char();
    std::cout << getResponseString("Go2API_Initialize", Go2Api_Initialize()) << std::endl;
    std::cout << getResponseString("Go2System_Construct", Go2System_Construct(&go2system)) << std::endl;
    std::cout << getResponseString("Go2System_Connect", Go2System_Connect(go2system, GO2_DEFAULT_IP_ADDRESS)) << std::endl;
    std::cout << getResponseString("Go2System_Login", Go2System_Login(go2system, user, password)) << std::endl;
    delete(password);
    return go2system;
}

void configEncoder(Go2System& go2system, double resolution, double travel_threshold) {
    std::cout << getResponseString("Go2System_SetTriggerSource", Go2System_SetTriggerSource(go2system, GO2_TRIGGER_SOURCE_ENCODER)) << std::endl;
    // Disable gate
    Go2Bool gate_trigger = GO2_FALSE;
    std::cout << getResponseString("Go2System_SetTravelResolution", Go2System_SetTravelResolution(go2system, resolution)) << std::endl;
    std::cout << getResponseString("Go2System_EnableTriggerGate", Go2System_EnableTriggerGate(go2system, gate_trigger)) << std::endl;
    std::cout << getResponseString("Go2System_SetEncoderPeriod", Go2System_SetEncoderPeriod(go2system, travel_threshold)) << std::endl;
    std::cout << getResponseString("Go2System_SetEncoderTriggerMode", Go2System_SetEncoderTriggerMode(go2system, GO2_ENCODER_TRIGGER_MODE_BIDIRECTIONAL)) << std::endl;
}

void recordProfile(Go2System& go2system, std::string& outputFilename, 
                   double resolution, double travel_threshold) {
    std::ofstream fidout;
    fidout.open(outputFilename.c_str(), std::ios_base::app);
    if (fidout.is_open()) {
        fidout << "# File format: X Position [mm], Y Position [mm], Z Range [mm]" << std::endl;
        Go2ProfileData data = GO2_NULL;
        Go2Data dataItem;
        Go2Int64 encoderCounter;
        unsigned int itemCount = 0;
        std::cout << getResponseString("Go2System_Start",Go2System_Start(go2system)) << std::endl;
        Go2Status returnCode = Go2System_ConnectData(go2system, GO2_NULL, GO2_NULL);
        std::cout << getResponseString("Go2System_ConnectData", returnCode) << std::endl;
        if (returnCode != GO2_OK) {
            std::cout << "Initialization failed, program halted." << std::endl;
        }
        while(true) {
            Go2Status returnCode = Go2System_ReceiveData(go2system, RECEIVE_TIMEOUT, &data);
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
                    std::cout << getResponseString("Go2System_GetEncoder", Go2System_GetEncoder(go2system, &encoderCounter)) << std::endl; // number of ticks of encoder
                    for(unsigned int arrayIndex=0;arrayIndex<profilePointCount; ++arrayIndex) {
                        if (profileData[arrayIndex] != INVALID_RANGE_16BIT) {
                            fidout << XOffset+XResolution*arrayIndex << "," << encoderCounter*resolution << "," << ZOffset+ZResolution*profileData[arrayIndex] << std::endl;
                        } else {
                            std::cout << "Invalid reading, skipped." << std::endl;
                        }
                    }
                }
            }
        }
        fidout.close();
    } else {
        std::cerr << "Unable to open/write to output file '" << outputFilename << ".'" << std::endl;
    }
}

void shutdownGo2(Go2System& go2system) {
    std::cout << getResponseString("Go2System_Stop",Go2System_Stop(go2system)) << std::endl;
    std::cout << getResponseString("Go2System_Logout", Go2System_Logout(go2system)) << std::endl;
    std::cout << getResponseString("Go2System_Destroy", Go2System_Destroy(go2system)) << std::endl;
}

int main(int argc, char* argv[]) {
    // Startup and login
    Go2System sys = initGo2();
    
    // Set encoder resolution and travel threshold
    std::cout << "\n\nUsing RLS LM10IC050 encoder:  resolution 50 microns, trigger every 100 microns" << std::endl;
    double resolution = 0.05; // 50 microns per "tick" on the RLS LM10IC050
    double travel_threshold = 2*resolution; // trigger every 0.1 mm
    configEncoder(sys, resolution, travel_threshold);

    // Output profile
    std::string outputFilename = "profile.csv";
    if (argc==2) {
        outputFilename = argv[1];
    }
    std::cout << "Saving profile data to '" << outputFilename << "'" << std::endl;
    recordProfile(sys, outputFilename, resolution, travel_threshold);
    
    // Logout and shutdown
    shutdownGo2(sys);
}