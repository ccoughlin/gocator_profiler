#pragma once
extern "C" {
    #include "Go2.h"
}
#include "go2response.h"
#include <iostream>
#include <string>
#include <cstring>

// Creates and initializes a Gocator 20x0 system.
// GocatorSystem go2system();
// go2system.init();
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
        GocatorSystem(bool verboseFlag=false):user(GO2_USER_ADMIN), sys(0), verbose(verboseFlag) {
            password = new Go2Char();
        }
        virtual ~GocatorSystem();

        void init();
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
    Go2User user;
    Go2Char* password;
    Go2System sys;
    bool verbose;
};