/* gocator_encoder - sample console application to use Gocator 20x0 laser profilers

Chris R. Coughlin (TRI/Austin, Inc.)
*/
extern "C" {
    #include "Go2.h"
}
#include "gocatorsystem.h"
#include "gocatorcontrol.h"

#include <iostream>
#include <string>

// Usage: gocator_encoder [outputfile]
// If not specified, writes X,Y,Z data to file 'profile.csv' in current folder.
int main(int argc, char* argv[]) {
    // Startup and login
    GocatorSystem gocator(true);
    GocatorControl control(gocator, true);
    gocator.init();

    // Set encoder resolution and travel threshold
    // Default to the lab's current encoder
    std::cout << "\n\nUsing RLS LM10IC050 encoder:  resolution 50 microns, trigger every 100 microns" << std::endl;
    double resolution = 0.05; // 50 microns per "tick" on the RLS LM10IC050
    double travel_threshold = 2*resolution; // trigger every 0.1 mm
    control.configureEncoder(resolution, travel_threshold);

    // Output profile
    std::string outputFilename = "profile.csv";
    if (argc==2) {
        outputFilename = argv[1];
    }
    std::cout << "Saving profile data to '" << outputFilename << "'" << std::endl;
    control.recordProfile(outputFilename);
    return (0);
}