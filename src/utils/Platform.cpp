#include "Platform.hpp"

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#endif

namespace dutils {

bool Platform::initializeSockets() {
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        return false;
    }
#endif
    return true;
}

void Platform::cleanupSockets() {
#ifdef _WIN32
    WSACleanup();
#endif
}

void Platform::closeSocket(int sock) {
#ifdef _WIN32
    if (sock != INVALID_SOCKET) {
        ::closesocket(sock);
    }
#else
    if (sock >= 0) {
        ::close(sock);
    }
#endif
}

int Platform::getLastError() {
#ifdef _WIN32
    return WSAGetLastError();
#else
    return errno;
#endif
}

} // namespace dutils
