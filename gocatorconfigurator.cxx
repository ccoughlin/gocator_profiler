#include "gocatorconfigurator.h"

namespace opts = boost::program_options;
namespace filesystem = boost::filesystem;

// Case insensitive char comparison, courtesy C++ Cookbook
inline bool compareChars(char a, char b) {
    return(toupper(a) == toupper(b));
}

bool compareStrings(const std::string& s1, const std::string& s2) {
    return ((s1.size()==s2.size()) &&
            std::equal(s1.begin(), s1.end(), s2.begin(), compareChars));
}

// Returns a configured encoder from the specified configuration file
Encoder GocatorConfigurator::configuredEncoder(std::string& configFile) {
    if (!filesystem::exists(configFile.c_str())) {
        std::cerr << "<< Unable to find configuration file '" << configFile << ",' aborting >>" << std::endl;
        throw std::runtime_error("Configuration file not found");
    }
    try {
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
    } catch (const boost::program_options::invalid_option_value& ex) {
        std::cerr << "Encountered a bad config option in '" << configFile << ".'" << std::endl;
        throw(ex);
    }
}

// Configures trigger for the specified GocatorControl
Trigger* GocatorConfigurator::configuredTrigger(std::string& configFile) {
    if (!filesystem::exists(configFile.c_str())) {
        std::cerr << "<< Unable to find configuration file '" << configFile << ",' aborting >>" << std::endl;
        throw std::runtime_error("Configuration file not found");
    }
    try {
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
    } catch (const boost::program_options::invalid_option_value& ex) {
        std::cerr << "Encountered a bad config option in '" << configFile << ".'" << std::endl;
        throw(ex);
    } 
}