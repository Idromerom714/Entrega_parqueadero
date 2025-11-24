#ifndef SOCKET_UTILS_HPP
#define SOCKET_UTILS_HPP

#include <string>

// Compatibilidad multiplataforma
#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
    typedef SOCKET socket_t;
    #define SOCKET_ERROR_CODE WSAGetLastError()
    #define CLOSE_SOCKET closesocket
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #include <cstring>
    typedef int socket_t;
    #define INVALID_SOCKET -1
    #define SOCKET_ERROR -1
    #define SOCKET_ERROR_CODE errno
    #define CLOSE_SOCKET close
#endif

// Inicializar sockets (solo necesario en Windows)
bool inicializar_sockets();

// Limpiar sockets (solo necesario en Windows)
void limpiar_sockets();

// Obtener último error
std::string obtener_error_socket();

#endif