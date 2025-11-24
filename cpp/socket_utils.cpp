#include "socket_utils.hpp"
#include <sstream>

#ifdef _WIN32
    #include <windows.h>
#else
    #include <errno.h>
#endif

bool inicializar_sockets() {
#ifdef _WIN32
    WSADATA wsaData;
    int resultado = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (resultado != 0) {
        return false;
    }
#endif
    return true;
}

void limpiar_sockets() {
#ifdef _WIN32
    WSACleanup();
#endif
}

std::string obtener_error_socket() {
#ifdef _WIN32
    int error = WSAGetLastError();
    std::stringstream ss;
    ss << "Error " << error;
    return ss.str();
#else
    return std::string(strerror(errno));
#endif
}