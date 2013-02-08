#include "gocatorconfigurator.h"

namespace opts = boost::program_options;

// Case insensitive char comparison, courtesy C++ Cookbook
inline bool compareChars(char a, char b) {
    return(toupper(a) == toupper(b));
}

bool compareStrings(const std::string& s1, const std::string& s2) {
    return ((s1.size()==s2.size()) &&
            std::equal(s1.begin(), s1.end(), s2.begin(), compareChars));
}

// Returns the device serial number from the config file
Go2UInt32 GocatorConfigurator::deviceID(std::string& configFile) {
    Go2UInt32 deviceID;
    std::ifstream fidin;
    fidin.open(configFile.c_str());
    if (fidin.is_open()) {
        opts::options_description opt_desc("Available options");
        opt_desc.add_options()
            ("System.device_id", opts::value<Go2UInt32>()->default_value(0000000), "Device Serial Number");
        opts::variables_map config;
        opts::store(opts::parse_config_file(fidin, opt_desc, true), config);
        opts::notify(config);
        deviceID = config["System.device_id"].as<Go2UInt32>();
    }
    return deviceID;  
}

// Returns a configured encoder from the specified configuration file
Encoder GocatorConfigurator::configuredEncoder(std::string& configFile) {
    Encoder configuredEncoder;
    std::ifstream fidin;
    fidin.open(configFile.c_str());
    if (fidin.is_open()) {
        opts::options_description opt_desc("Available options");
        opt_desc.add_options()
            ("Encoder.model", opts::value<std::string>()->default_value("<unspecified>"), "Make/model of encoder")
            ("Encoder.resolution", opts::value<double>()->default_value(0), "Resolution of encoder [mm per 'tick']")
            ("Encoder.travel_threshold", opts::value<double>()->default_value(0), "Desired trigger threshold [mm]")
            ("Encoder.travel_direction", opts::value<std::string>()->default_value("bidirectional"), "Trigger direction");
        opts::variables_map config;
        opts::store(opts::parse_config_file(fidin, opt_desc, true), config);
        opts::notify(config);
        configuredEncoder.modelName = config["Encoder.model"].as<std::string>();
        configuredEncoder.resolution = config["Encoder.resolution"].as<double>();
        assert(configuredEncoder.resolution > 0);
    }
    return configuredEncoder;
}

// Configures trigger for the specified GocatorControl
Trigger* GocatorConfigurator::configuredTrigger(std::string& configFile) {
    std::ifstream fidin;
    fidin.open(configFile.c_str());
    if (fidin.is_open()) {
        opts::options_description opt_desc("Available options");
        opt_desc.add_options()
            ("Trigger.type", opts::value<std::string>()->default_value("Time"), "Type of trigger")
            ("Trigger.frame_rate", opts::value<double>()->default_value(0), "Frame rate of time trigger [Hz]")
            ("Trigger.travel_threshold", opts::value<double>()->default_value(0), "Desired trigger threshold [mm]")
            ("Trigger.travel_direction", opts::value<std::string>()->default_value("bidirectional"), "Trigger direction")
            ("Trigger.enable_gate", opts::value<std::string>()->default_value("false"), "Enable trigger gates");
        opts::variables_map config;
        opts::store(opts::parse_config_file(fidin, opt_desc, true), config);
        opts::notify(config);
        std::string triggerType = config["Trigger.type"].as<std::string>();
        if (compareStrings(triggerType, "encoder")) {
            EncoderTrigger trig;                
            trig.setTravelThreshold(config["Trigger.travel_threshold"].as<double>());
            std::string travel_direction = config["Trigger.travel_direction"].as<std::string>();
            enum TravelDirection travel_dir;
            if (compareStrings(travel_direction, "forward")) {
                travel_dir = FORWARD;
            } else if (compareStrings(travel_direction, "backward")) {
                travel_dir = BACKWARD;
            } else {
                travel_dir = BIDIRECTIONAL;
            }
            trig.setTravelDirection(travel_dir);
            bool enableGate = false;
            if (compareStrings(config["Trigger.enable_gate"].as<std::string>(), "true")) {
                enableGate = true;
            }
            trig.setTriggerGate(enableGate);
            return new EncoderTrigger(trig);
        } else if (compareStrings(triggerType, "Input")) {
            InputTrigger trig;
            bool enableGate = false;
            if (compareStrings(config["Trigger.enable_gate"].as<std::string>(), "true")) {
                enableGate = true;
            }
            trig.setTriggerGate(enableGate);
            return new InputTrigger(trig);
        } else if (compareStrings(triggerType, "Software")) {
            SoftwareTrigger trig;
            bool enableGate = false;
            if (compareStrings(config["Trigger.enable_gate"].as<std::string>(), "true")) {
                enableGate = true;
            }
            trig.setTriggerGate(enableGate);
            return new SoftwareTrigger(trig);
        } else { // default to time triggering
            TimeTrigger trig;
            trig.setFrameRate(config["Trigger.frame_rate"].as<double>());
            bool enableGate = false;
            if (compareStrings(config["Trigger.enable_gate"].as<std::string>(), "true")) {
                enableGate = true;
            }
            trig.setTriggerGate(enableGate);
            return new TimeTrigger(trig);
        }
    }
    return NULL;
}

// Returns a Go2AddressInfo (Ethernet connection) from the config file
GocatorAddress GocatorConfigurator::configuredNetworkConnection(std::string& configFile) {
    GocatorAddress GoAddress;
    Go2AddressInfo configuredAddress;
    std::ifstream fidin;
    fidin.open(configFile.c_str());
    if (fidin.is_open()) {
        opts::options_description opt_desc("Available options");
        opt_desc.add_options()
            ("Network.reconfigure", opts::value<std::string>()->default_value("false"), "Reconfigure?")
            ("Network.use_dhcp", opts::value<std::string>()->default_value("false"), "Use DHCP?")
            ("Network.address", opts::value<std::string>()->default_value("192.168.1.10"), "IP Address")
            ("Network.subnet_mask", opts::value<std::string>()->default_value("255.255.255.0"), "Subnet Mask")
            ("Network.gateway", opts::value<std::string>()->default_value("0.0.0.0"), "Gateway");
        opts::variables_map config;
        opts::store(opts::parse_config_file(fidin, opt_desc, true), config);
        opts::notify(config);
        std::string reconfigureAddress = config["Network.reconfigure"].as<std::string>();
        GoAddress.reconfigure = compareStrings(reconfigureAddress, "true");
        std::string useDHCP = config["Network.use_dhcp"].as<std::string>();
        if(compareStrings(useDHCP, "true")) {
            configuredAddress.useDhcp = true;
        } else {
            configuredAddress.useDhcp = false;
        }
        std::string networkAddress = config["Network.address"].as<std::string>();
        Go2IPAddress_Parse(reinterpret_cast<const signed char*>(networkAddress.c_str()), &configuredAddress.address);
        std::string netMask = config["Network.subnet_mask"].as<std::string>();
        Go2IPAddress_Parse(reinterpret_cast<const signed char*>(netMask.c_str()), &configuredAddress.mask);
        std::string gateway = config["Network.gateway"].as<std::string>();
        Go2IPAddress_Parse(reinterpret_cast<const signed char*>(gateway.c_str()), &configuredAddress.gateway);
    }
    GoAddress.addr = configuredAddress;
    return GoAddress;
}

// Returns the filtration options from the config file
GocatorFilter GocatorConfigurator::configuredFilter(std::string& configFile) {
    GocatorFilter filter;
    std::ifstream fidin;
    fidin.open(configFile.c_str());
    if (fidin.is_open()) {
        opts::options_description opt_desc("Available options");
        opt_desc.add_options()
            ("Filtering.scanner_resolution", opts::value<std::string>()->default_value("medium"), "Sampling Resolution")
            ("Filtering.xgap_fill", opts::value<double>()->default_value(0), "Horizontal gap filling")
            ("Filtering.ygap_fill", opts::value<double>()->default_value(0), "Vertical gap filling")
            ("Filtering.xsmooth", opts::value<double>()->default_value(0), "Horizontal signal averaging")
            ("Filtering.ysmooth", opts::value<double>()->default_value(0), "Vertical signal averaging");
        opts::variables_map config;
        opts::store(opts::parse_config_file(fidin, opt_desc, true), config);
        opts::notify(config);
        std::string resolution = config["Filtering.scanner_resolution"].as<std::string>();
        if(compareStrings(resolution, "low")) {
            filter.sampling = GO2_RESAMPLING_TYPE_MAX_SPEED;
        } else if (compareStrings(resolution, "high")) {
            filter.sampling = GO2_RESAMPLING_TYPE_MAX_RES;
        } else {
            filter.sampling = GO2_RESAMPLING_TYPE_BALANCED;
        }

        double xgap = config["Filtering.xgap_fill"].as<double>();
        if (xgap > 0) {
            filter.xGapWindow = xgap;
            filter.xGap = true;
        } else {
            filter.xGapWindow = 0;
            filter.xGap = false;
        }
        double ygap = config["Filtering.ygap_fill"].as<double>();
        if (ygap > 0) {
            filter.yGapWindow = ygap;
            filter.yGap = true;
        } else {
            filter.yGapWindow = 0;
            filter.yGap = false;
        }

        double xsmooth = config["Filtering.xsmooth"].as<double>();
        if (xsmooth > 0) {
            filter.xSmoothWindow = xsmooth;
            filter.xSmooth = true;
        } else {
            filter.xSmoothWindow = 0;
            filter.xSmooth = false;
        }
        double ysmooth = config["Filtering.ysmooth"].as<double>();
        if (ysmooth > 0) {
            filter.ySmoothWindow = ysmooth;
            filter.ySmooth = true;
        } else {
            filter.ySmoothWindow = 0;
            filter.ySmooth = false;
        }
    }
    return filter; 
}