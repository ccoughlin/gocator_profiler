#include "gocatorsystem.h"

GocatorSystem::~GocatorSystem() {
    delete(password);
    if (sys!=0) {
        std::string StopSystemResponse, LogoutResponse, SystemDestroyResponse;
        StopSystemResponse = getResponseString("Go2System_Stop",Go2System_Stop(sys));
        LogoutResponse = getResponseString("Go2System_Logout", Go2System_Logout(sys));
        SystemDestroyResponse = getResponseString("Go2System_Destroy", Go2System_Destroy(sys));
        if(verbose) {
            std::cout << StopSystemResponse << std::endl;
            std::cout << LogoutResponse << std::endl;
            std::cout << SystemDestroyResponse << std::endl;
        }
    }
}

// Initializes specified Gocator device
void GocatorSystem::init(Go2UInt32 deviceID, Go2AddressInfo desiredNetworkAddress) {
    int MAX_SENSORS = 10;
    std::string InitializeResponse, ConstructResponse, DiscoverResponse, SetAddressResponse, ConnectResponse, LoginResponse;
    InitializeResponse = getResponseString("Go2API_Initialize", Go2Api_Initialize());
    ConstructResponse = getResponseString("Go2System_Construct", Go2System_Construct(&sys));
    bool foundDevice = false, resetIP = false;
    /* Look for the requested device - if found and its address doesn't match the desired,
    reset and reconnect. */
    Go2UInt32* deviceIDs[MAX_SENSORS];
    Go2AddressInfo* deviceAddresses[MAX_SENSORS];
    Go2UInt32 numDevices;
    DiscoverResponse = getResponseString("Go2System_Discover", 
                                         Go2System_Discover(deviceIDs, deviceAddresses, &numDevices));
    if (verbose) {
        std::cout << DiscoverResponse << std::endl;
    }
    unsigned int i;
    Go2UInt32* currentDevice;
    for(i=0;i<numDevices;i++) {
        currentDevice = deviceIDs[i];
        if (*currentDevice==deviceID) {
            Go2AddressInfo* deviceAddress = deviceAddresses[i];
            if(deviceAddress->useDhcp!=desiredNetworkAddress.useDhcp ||
               deviceAddress->address!=desiredNetworkAddress.address ||
               deviceAddress->mask!=desiredNetworkAddress.mask ||
               deviceAddress->gateway!=desiredNetworkAddress.gateway) {
                SetAddressResponse = getResponseString("Go2System_SetAddress",
                                                       Go2System_SetAddress(deviceID, &desiredNetworkAddress));
                if (verbose) {
                    std::cout << SetAddressResponse << std::endl;
                }
                Go2System_Reset(sys);
                resetIP = true;
            }
            foundDevice = true;
            break;
        }    
    }
    for (i=0;i<numDevices;i++) {
        Go2Free((void *)deviceIDs[i]);
        Go2Free((void *)deviceAddresses[i]);
    }
    if (!foundDevice) {
        std::cerr << "\n<< Unable to detect device #" << deviceID << ", aborting >>" << std::endl;
        throw std::runtime_error("Device not found");
    }
    if (resetIP) {
        // Need to wait for device to reboot with new settings
        ConnectResponse = getResponseString("Go2System_Reconnect", Go2System_Reconnect(sys, desiredNetworkAddress.address));
    } else {
        ConnectResponse = getResponseString("Go2System_Connect", Go2System_Connect(sys, desiredNetworkAddress.address));        
    }
    LoginResponse = getResponseString("Go2System_Login", Go2System_Login(sys, user, password));
    if (verbose) {
        std::cout << InitializeResponse << std::endl;
        std::cout << ConstructResponse << std::endl;
        std::cout << ConnectResponse << std::endl;
        std::cout << LoginResponse << std::endl;
    }
}