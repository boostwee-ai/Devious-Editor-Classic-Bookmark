#pragma once

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
