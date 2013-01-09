/* gocator_encoder - sample console application to use Gocator 20x0 laser profilers

Chris R. Coughlin (TRI/Austin, Inc.)
*/
extern "C" {
    #include "Go2.h"
}
#include "gocatorsystem.h"
#include "gocatorcontrol.h"

#include <boost/program_options.hpp>
#include <iostream>
#include <string>

namespace opts = boost::program_options;

// Returns a configured linear encoder from the specified configuration file
Encoder configureEncoder(std::string& configFile) {
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
            if (travel_direction=="forward") {
                trig_dir = FORWARD;
            } else if (travel_direction=="backward") {
                trig_dir = BACKWARD;
            } else {
                trig_dir = BIDIRECTIONAL;
            }
            configuredEncoder.trigger_direction = trig_dir;
        }
    } catch (const boost::program_options::invalid_option_value& ex) {
        std::cout << "Encountered a bad config option in '" << configFile << ".'" << std::endl;
        throw(ex);
    }
    return configuredEncoder;
}

// Usage: gocator_encoder [--output outputfile] [--config configfile]
// If not specified, writes X,Y,Z data to file 'profile.csv' in current folder.
int main(int argc, char* argv[]) {

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
        std::cout << "\nSaving profile data to '" << outputFilename << "'" << std::endl;
    }

    std::string configFilename;
    if (cmdline.count("config")) {
        configFilename = cmdline["config"].as<std::string>();
    }
    if (verbose) {
        std::cout << "\nAdditional config read from '" << configFilename << "'" << std::endl;  
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
                trig_dir = "unidirectional";
        }
        std::cout << "\tTrigger on " << trig_dir << " movement of more than " << lme.travel_threshold << " mm>>\n" << std::endl;
    }
    
    // Output profile    
    control.recordProfile(outputFilename);
    return 0;
}