/* gocator_encoder - sample console application to use Gocator 20x0 laser profilers

Chris R. Coughlin (TRI/Austin, Inc.)
*/
extern "C" {
    #include "Go2.h"
}
#include "gocatorsystem.h"
#include "gocatorcontrol.h"

#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include <boost/thread/thread.hpp>
#include <algorithm>
#include <cctype>
#include <iostream>
#include <string>

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

// Returns a configured linear encoder from the specified configuration file
Encoder configureEncoder(std::string& configFile) {
    if (!filesystem::exists(configFile.c_str())) {
        std::cerr << "<< Unable to find configuration file '" << configFile << ",' aborting >>" << std::endl;
        throw std::runtime_error("Configuration file not found");
    }
    Encoder configuredEncoder;
    std::ifstream fidin;
    fidin.open(configFile.c_str());
    try {
        if (fidin.is_open()) {
            opts::options_description opt_desc("Available options");
            opt_desc.add_options()
                ("Encoder.model", opts::value<std::string>()->default_value("<unspecified>"), "Make/model of encoder")
                ("Encoder.resolution", opts::value<double>()->default_value(0), "Resolution of encoder [mm per 'tick']")
                ("Encoder.travel_threshold", opts::value<double>()->default_value(0), "Desired trigger threshold [mm]")
                ("Encoder.travel_direction", opts::value<std::string>()->default_value("bidirectional"), "Trigger direction");
            opts::variables_map config;
            opts::store(opts::parse_config_file(fidin, opt_desc), config);
            opts::notify(config);
            configuredEncoder.modelName = config["Encoder.model"].as<std::string>();
            configuredEncoder.resolution = config["Encoder.resolution"].as<double>();
            assert(configuredEncoder.resolution > 0);
            configuredEncoder.travel_threshold = config["Encoder.travel_threshold"].as<double>();
            if (configuredEncoder.travel_threshold < configuredEncoder.resolution) {
                std::cerr << "*** Invalid travel_threshold, setting to " << 2*configuredEncoder.resolution << " mm." << std::endl;
                configuredEncoder.travel_threshold = 2*configuredEncoder.resolution;
            }
            enum TriggerDirection trig_dir;
            std::string travel_direction = config["Encoder.travel_direction"].as<std::string>();
            if (compareStrings(travel_direction, "forward")) {
                trig_dir = FORWARD;
            } else if (compareStrings(travel_direction, "backward")) {
                trig_dir = BACKWARD;
            } else {
                trig_dir = BIDIRECTIONAL;
            }
            configuredEncoder.trigger_direction = trig_dir;
        }
    } catch (const boost::program_options::invalid_option_value& ex) {
        std::cerr << "Encountered a bad config option in '" << configFile << ".'" << std::endl;
        throw(ex);
    }
    return configuredEncoder;
}

void recordProfile(GocatorControl& control, std::string& outputFilename) {
    boost::thread thd(boost::bind(&GocatorControl::recordProfile, control, outputFilename));
    char character2;
    std::cout << "Press any key + Enter to stop recording." << std::endl;
    std::cin >> character2;
    thd.interrupt();    
    thd.join();
}

// Usage: gocator_encoder [--output outputfile] [--config configfile]
// If not specified, writes X,Y,Z data to file 'profile.csv' in current folder.
int main(int argc, char* argv[]) {
    std::cout << "Gocator 20x0 Profiler" << std::endl;
    std::cout << "Chris R. Coughlin (TRI/Austin, Inc.)" << std::endl;
    opts::options_description opt_desc("Available options");
    opt_desc.add_options()
        ("output,o", opts::value<std::string>()->default_value("profile.csv"), "output file for profile data")
        ("config,c", opts::value<std::string>()->default_value("gocator_encoder.cfg"), "configuration file")
        ("help,h", "display basic help information")
        ("verbose,v", "display additional messages")
    ;

    // Parse command line
    opts::variables_map cmdline;
    opts::store(opts::parse_command_line(argc, argv, opt_desc), cmdline);
    opts::notify(cmdline);
    if (cmdline.count("help")) {
        std::cout << opt_desc << std::endl;
        return 1;
    }
    bool verbose = false;
    if (cmdline.count("verbose")) {
        verbose = true;
    }
    std::string outputFilename;
    if (cmdline.count("output")) {
        outputFilename = cmdline["output"].as<std::string>();
    }
    if (verbose) {
        std::cout << "Saving profile data to '" << outputFilename << "'" << std::endl;
    }

    std::string configFilename;
    if (cmdline.count("config")) {
        configFilename = cmdline["config"].as<std::string>();
    }
    if (verbose) {
        std::cout << "Additional config read from '" << configFilename << "'\n\n" << std::endl;  
    }

    // Startup and login
    GocatorSystem gocator(verbose);
    GocatorControl control(gocator, verbose);
    gocator.init();

    // Configure encoder 
    Encoder lme = configureEncoder(configFilename);
    control.configureEncoder(lme);
    if (verbose) {
        std::cout << "\n<< Using " << lme.modelName << " encoder:" << std::endl;
        std::cout << "\tResolution " << lme.resolution << " mm/tick" << std::endl;
        std::string trig_dir;
        switch(lme.trigger_direction) {
            case FORWARD:
                trig_dir = "forward";
                break;
            case BACKWARD:
                trig_dir = "backward";
                break;
            default:
                trig_dir = "bidirectional";
        }
        std::cout << "\tTrigger on " << trig_dir << " movement of more than " << lme.travel_threshold << " mm>>\n" << std::endl;
    }
    
    // Output profile  
    std::cout << "Connected to Gocator, monitoring encoder..." << std::endl;  
    //control.recordProfile(outputFilename);
    recordProfile(control, outputFilename);
    return 0;
}