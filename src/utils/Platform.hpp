#pragma once

#include <string>
#include <vector>
#include <cstdint>

namespace utils {

class Platform {
public:
    static bool initializeSockets();
    static void cleanupSockets();
    static void closeSocket(int sock);
    static int getLastError();
};

} // namespace utils
