/* gocator_encoder - sample console application to use Gocator 20x0 laser profilers

Chris R. Coughlin (TRI/Austin, Inc.)
*/
extern "C" {
    #include "Go2.h"
}
#include "gocatorsystem.h"
#include "gocatorcontrol.h"
#include "gocatorconfigurator.h"

#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include <boost/thread/thread.hpp>
#include <algorithm>
#include <cctype>
#include <iostream>
#include <string>

namespace opts = boost::program_options;
namespace filesystem = boost::filesystem;

// Thread wrapper to run profile recording
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
    Encoder lme = GocatorConfigurator::configuredEncoder(configFilename);
    control.configureEncoder(lme);
    if (verbose) {
        std::cout << "\n<< Using " << lme.modelName << " encoder, ";
        std::cout << "resolution " << lme.resolution << " mm/tick" << std::endl;
    }
    
    // Configure trigger
    boost::shared_ptr<Trigger> trigger(GocatorConfigurator::configuredTrigger(configFilename));
    if (verbose) {
        std::cout << "<< Triggering: " << trigger->getTriggerType() << ", gate ";
        if (trigger->isTriggerGateEnabled()) {
            std::cout << "enabled";
        } else {
            std::cout << "disabled";
        }
        std::cout << " >>\n\n" << std::endl;
    }
    trigger->set(control);
    // Output profile  
    std::cout << "Connected to Gocator, monitoring encoder..." << std::endl;  
    recordProfile(control, outputFilename);
    return 0;
}