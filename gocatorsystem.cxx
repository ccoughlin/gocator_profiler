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

void GocatorSystem::init() {
    std::string InitializeResponse, ConstructResponse, ConnectResponse, LoginResponse;
    InitializeResponse = getResponseString("Go2API_Initialize", Go2Api_Initialize());
    ConstructResponse = getResponseString("Go2System_Construct", Go2System_Construct(&sys));
    ConnectResponse = getResponseString("Go2System_Connect", Go2System_Connect(sys, GO2_DEFAULT_IP_ADDRESS));
    LoginResponse = getResponseString("Go2System_Login", Go2System_Login(sys, user, password));
    if (verbose) {
        std::cout << InitializeResponse << std::endl;
        std::cout << ConstructResponse << std::endl;
        std::cout << ConnectResponse << std::endl;
        std::cout << LoginResponse << std::endl;
    }
}