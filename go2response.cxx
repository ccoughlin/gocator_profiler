#include "go2response.h"

std::string getResponseString(std::string function, Go2Status returnCode) {
    std::string responseString = "<< " + function + " response: ";
    switch(returnCode) {
        case GO2_ERROR:
            responseString += "general error >>";
            break;
        case GO2_ERROR_ABORT:
            responseString += "operation aborted >>";
            break;
        case GO2_ERROR_ALREADY_EXISTS:
            responseString += "conflicts with existing item >>";
            break;
        case GO2_ERROR_CLOSED:
            responseString += "resource no longer available >>";
            break;
        case GO2_ERROR_COMMAND:
            responseString += "command not recognized >>";
            break;
        case GO2_ERROR_HANDLE:
            responseString += "handle is invalid >>";
            break;
        case GO2_ERROR_INCOMPLETE:
            responseString += "buffer not large enough for data >>";
            break;
        case GO2_ERROR_MEMORY:
            responseString += "out of memory >>";
            break;
        case GO2_ERROR_NOT_FOUND:
            responseString += "item not found >>";
            break;
        case GO2_ERROR_PARAMETER:
            responseString += "parameter is invalid >>";
            break;
        case GO2_ERROR_STATE:
            responseString += "invalid state >>";
            break;
        case GO2_ERROR_STREAM:
            responseString += "error in stream >>";
            break;
        case GO2_ERROR_TIMEOUT:
            responseString += "action timed out >>";
            break;
        case GO2_ERROR_UNIMPLEMENTED:
            responseString += "feature not implemented >>";
            break;
        case GO2_ERROR_VERSION:
            responseString += "invalid version number >>";
            break;
        case GO2_OK:
            responseString += "ok >>";
            break;    
    }
    return responseString;
}