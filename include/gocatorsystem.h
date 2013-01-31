#pragma once
extern "C" {
    #include "Go2.h"
}
#include "go2response.h"
#include <iostream>
#include <string>
#include <stdexcept>
#include <cstring>

// Creates and initializes a Gocator 20x0 system.
// GocatorSystem go2system();
// go2system.init(deviceSerialNumber);
// Return the created and logged-in system: go2system.getSystem();
class GocatorSystem {
    public:
        GocatorSystem(std::string& pword, Go2User go2user, bool verboseFlag=false):
        user(go2user), sys(0), verbose(verboseFlag) {
            setPassword(pword);
        }
        GocatorSystem(std::string& pword, bool verboseFlag=false):
        user(GO2_USER_ADMIN), sys(0), verbose(verboseFlag) {
            setPassword(pword);
        }
        GocatorSystem(bool verboseFlag=false):
        user(GO2_USER_ADMIN), sys(0), verbose(verboseFlag) {
            password = new Go2Char();
        }
        virtual ~GocatorSystem();

        void init(Go2UInt32 deviceID, 
                  Go2AddressInfo desiredNetworkAddress=defaultGocatorAddress(),
                  bool reconfigureAddress=false);
        Go2User getUser() {
            return user;
        }
        Go2System& getSystem() {
            return sys;
        }
        void setPassword(std::string& pword) {
            delete(password);
            password = new Go2Char(pword.length() + 1);
            memcpy(password, pword.c_str(), strlen(pword.c_str()));
        }

    private:
        static Go2AddressInfo defaultGocatorAddress() {
            Go2IPAddress defaultMask, defaultGateway;
            Go2IPAddress_Parse(reinterpret_cast<const signed char*>("255.255.255.0"), &defaultMask);
            Go2IPAddress_Parse(reinterpret_cast<const signed char*>("0.0.0.0"), &defaultGateway);
            Go2AddressInfo defaultAddress;
            defaultAddress.useDhcp = false;
            defaultAddress.address = GO2_DEFAULT_IP_ADDRESS;
            defaultAddress.mask = defaultMask;
            defaultAddress.gateway = defaultGateway;
            return defaultAddress;            
        }
    Go2User user;
    Go2Char* password;
    Go2System sys;
    bool verbose;
};