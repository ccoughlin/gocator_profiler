#include "gocatorcontrol.h"
namespace filesystem = boost::filesystem;
namespace posixtime = boost::posix_time;

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
    resetEncoder();
}

// Configures Gocator's filtration
void GocatorControl::configureFilter(GocatorFilter& filter) {
    std::string setSamplingResponse = getResponseString("Go2System_SetXResamplingType", 
                                                        Go2System_SetXResamplingType(sys.getSystem(), filter.sampling));
    std::string setHGapFillResponse = getResponseString("Go2System_SetXGapFillingEnabled",
                                                        Go2System_SetXGapFillingEnabled(sys.getSystem(), filter.xGap));
    if (filter.xGap) {
        Go2Double hGap;
        Go2Double hGapMax = Go2System_XGapFillingWindowMax(sys.getSystem());
        Go2Double hGapMin = Go2System_XGapFillingWindowMin(sys.getSystem());
        if (filter.xGap > hGapMax) {
            hGap = hGapMax;
        } else if (filter.xGap < hGapMin) {
            hGap = hGapMin;
        }
        setHGapFillResponse = Go2System_SetXGapFillingWindow(sys.getSystem(), hGap);
    }
    if (verbose) {
        std::cout << setHGapFillResponse << std::endl;
    }

    std::string setVGapFillResponse = getResponseString("Go2System_SetYGapFillingEnabled",
                                                        Go2System_SetYGapFillingEnabled(sys.getSystem(), filter.yGap));
    if (filter.yGap) {
        Go2Double vGap;
        Go2Double vGapMax = Go2System_YGapFillingWindowMax(sys.getSystem());
        Go2Double vGapMin = Go2System_YGapFillingWindowMin(sys.getSystem());
        if (filter.yGap > vGapMax) {
            vGap = vGapMax;
        } else if (filter.xGap < vGapMin) {
            vGap = vGapMin;
        }
        setVGapFillResponse = Go2System_SetYGapFillingWindow(sys.getSystem(), vGap);
    }
    if (verbose) {
        std::cout << setVGapFillResponse << std::endl;
    }


    std::string setHSmoothResponse = getResponseString("Go2System_SetXSmoothingEnabled",
                                                            Go2System_SetXSmoothingEnabled(sys.getSystem(), filter.xSmooth));
    if (filter.xSmooth) {
        Go2Double hSmooth;
        Go2Double hSmoothMax = Go2System_XSmoothingWindowMax(sys.getSystem());
        Go2Double hSmoothMin = Go2System_XSmoothingWindowMin(sys.getSystem());
        if (filter.xSmooth > hSmoothMax) {
            hSmooth = hSmoothMax;
        } else if (filter.xSmooth < hSmoothMin) {
            hSmooth = hSmoothMin;
        }
        setHSmoothResponse = Go2System_SetXSmoothingWindow(sys.getSystem(), hSmooth);
    }
    if (verbose) {
        std::cout << setHSmoothResponse << std::endl;
    }

    std::string setVSmoothResponse = getResponseString("Go2System_SetYSmoothingEnabled",
                                                                Go2System_SetYSmoothingEnabled(sys.getSystem(), filter.ySmooth));
    if (filter.ySmooth) {
        Go2Double vSmooth;
        Go2Double vSmoothMax = Go2System_YSmoothingWindowMax(sys.getSystem());
        Go2Double vSmoothMin = Go2System_YSmoothingWindowMin(sys.getSystem());
        if (filter.ySmooth > vSmoothMax) {
            vSmooth = vSmoothMax;
        } else if (filter.ySmooth < vSmoothMin) {
            vSmooth = vSmoothMin;
        }
        setVSmoothResponse = Go2System_SetYSmoothingWindow(sys.getSystem(), vSmooth);
    }
    if (verbose) {
        std::cout << setVSmoothResponse << std::endl;
    }
}

// Turns the laser on to allow positioning prior to beginning a scan
void GocatorControl::targetOn() {
    Go2System_SetTriggerSource(sys.getSystem(), GO2_TRIGGER_SOURCE_TIME);
    std::string StartResponse = getResponseString("Go2System_Start",Go2System_Start(sys.getSystem()));
    if (verbose) {
        std::cout << StartResponse << std::endl;
    }
}

// Turns the laser off
void GocatorControl::targetOff() {
    std::string StopResponse = getResponseString("Go2System_Stop", Go2System_Stop(sys.getSystem()));
    if (verbose) {
        std::cout << StopResponse << std::endl;
    }
}

void GocatorControl::recordProfile(std::string& outputFilename) {
    std::ostringstream tsStream;
    const posixtime::ptime now = posixtime::second_clock::local_time();
    posixtime::time_facet* const f = new posixtime::time_facet("%H:%M:%S %Y%b%d");
    tsStream.imbue(std::locale(tsStream.getloc(), f));
    tsStream << now;
    std::string msgString = "Scan Initiated " + tsStream.str();
    recordProfile(outputFilename, msgString);
}
    
// Records range profiles to disk as comma-delimited ASCII.
// The specified string is written into the header of the data file.
void GocatorControl::recordProfile(std::string& outputFilename, std::string& commentString) {
    try {
        filesystem::remove(outputFilename.c_str());
    } catch (filesystem::filesystem_error &err) {
        std::cerr << "<< Unable to overwrite '" << outputFilename << ",' appending >>" << std::endl;
    }
    std::ofstream fidout;
    fidout.open(outputFilename.c_str(), std::ios_base::app);
    if (fidout.is_open()) {
        fidout << "# File format: X Position [mm], Y Position [mm], Z Range [mm]" << std::endl;
        fidout << "# " << commentString << std::endl;
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
            boost::this_thread::interruption_point();
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
                    // Disable thread interruption while we're writing data
                    boost::this_thread::disable_interruption di;
                    // number of ticks of encoder
                    std::string GetEncoderResponse = getResponseString("Go2System_GetEncoder", Go2System_GetEncoder(sys.getSystem(), &encoderCounter));
                    if (verbose) {
                        std::cout << GetEncoderResponse << std::endl; 
                    }
                    for(unsigned int arrayIndex=0;arrayIndex<profilePointCount; ++arrayIndex) {
                        if (profileData[arrayIndex] != INVALID_RANGE_16BIT) {
                            fidout << XOffset+XResolution*arrayIndex << "," << (encoderCounter-startingEncoderReading)*lme.resolution << "," << ZOffset+ZResolution*profileData[arrayIndex] << std::endl << std::flush;
                        } else {
                            if (verbose) {
                                std::cout << "Invalid reading, skipped." << std::endl;
                            }
                        }
                    }
                    boost::this_thread::restore_interruption ri(di);
                }
            }
            boost::this_thread::yield();
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