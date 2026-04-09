#pragma once

#ifdef _WIN32
    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif
    #ifndef _WINSOCKAPI_
        #define _WINSOCKAPI_
    #endif
#endif

#include <string>
#include <vector>
#include <cstdint>

namespace dutils {

class Platform {
public:
    static bool initializeSockets();
    static void cleanupSockets();
    static void closeSocket(int sock);
    static int getLastError();
};

} // namespace dutils
