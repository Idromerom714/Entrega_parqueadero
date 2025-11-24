#include "servidor_parqueadero.hpp"
#include <iostream>
#include <sstream>

ServidorParqueadero::ServidorParqueadero(Parqueadero* p, int puerto)
    : parqueadero(p), puerto(puerto), servidor_socket(INVALID_SOCKET), ejecutando(false) {
}

ServidorParqueadero::~ServidorParqueadero() {
    detener();
}

bool ServidorParqueadero::iniciar() {
    if (!inicializar_sockets()) {
        std::cerr << "Error al inicializar sockets" << std::endl;
        return false;
    }
    
    // Crear socket
    servidor_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (servidor_socket == INVALID_SOCKET) {
        std::cerr << "Error al crear socket: " << obtener_error_socket() << std::endl;
        limpiar_sockets();
        return false;
    }
    
    // Permitir reutilizar dirección
    int opt = 1;
#ifdef _WIN32
    setsockopt(servidor_socket, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt));
#else
    setsockopt(servidor_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
#endif
    
    // Configurar dirección
    struct sockaddr_in direccion;
    direccion.sin_family = AF_INET;
    direccion.sin_addr.s_addr = INADDR_ANY;
    direccion.sin_port = htons(puerto);
    
    // Bind
    if (bind(servidor_socket, (struct sockaddr*)&direccion, sizeof(direccion)) == SOCKET_ERROR) {
        std::cerr << "Error en bind: " << obtener_error_socket() << std::endl;
        CLOSE_SOCKET(servidor_socket);
        limpiar_sockets();
        return false;
    }
    
    // Listen
    if (listen(servidor_socket, 5) == SOCKET_ERROR) {
        std::cerr << "Error en listen: " << obtener_error_socket() << std::endl;
        CLOSE_SOCKET(servidor_socket);
        limpiar_sockets();
        return false;
    }
    
    ejecutando = true;
    std::cout << "✅ Servidor iniciado en puerto " << puerto << std::endl;
    return true;
}

void ServidorParqueadero::detener() {
    if (ejecutando) {
        ejecutando = false;
        if (servidor_socket != INVALID_SOCKET) {
            CLOSE_SOCKET(servidor_socket);
            servidor_socket = INVALID_SOCKET;
        }
        limpiar_sockets();
        std::cout << "🛑 Servidor detenido" << std::endl;
    }
}

bool ServidorParqueadero::aceptar_conexion() {
    if (!ejecutando) {
        return false;
    }
    
    struct sockaddr_in direccion_cliente;
#ifdef _WIN32
    int addrlen = sizeof(direccion_cliente);
#else
    socklen_t addrlen = sizeof(direccion_cliente);
#endif
    
    std::cout << "⏳ Esperando conexión de dispositivo..." << std::endl;
    
    socket_t cliente_socket = accept(servidor_socket, 
                                     (struct sockaddr*)&direccion_cliente, 
                                     &addrlen);
    
    if (cliente_socket == INVALID_SOCKET) {
        std::cerr << "Error en accept: " << obtener_error_socket() << std::endl;
        return false;
    }
    
    char ip_cliente[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &direccion_cliente.sin_addr, ip_cliente, INET_ADDRSTRLEN);
    std::cout << "📡 Dispositivo conectado desde " << ip_cliente << std::endl;
    
    manejar_cliente(cliente_socket);
    CLOSE_SOCKET(cliente_socket);
    
    return true;
}

void ServidorParqueadero::manejar_cliente(socket_t cliente_socket) {
    char buffer[1024] = {0};
    
    // Recibir datos
    int bytes_recibidos = recv(cliente_socket, buffer, sizeof(buffer) - 1, 0);
    
    if (bytes_recibidos <= 0) {
        std::cerr << "Error al recibir datos" << std::endl;
        return;
    }
    
    buffer[bytes_recibidos] = '\0';
    std::string datos(buffer);
    
    std::cout << "📨 Mensaje recibido: " << datos << std::endl;
    
    // Parsear y procesar
    MensajeDispositivo mensaje = parsear_mensaje(datos);
    std::string respuesta = procesar_comando(mensaje);
    
    // Enviar respuesta
    send(cliente_socket, respuesta.c_str(), respuesta.length(), 0);
    std::cout << "📤 Respuesta enviada: " << respuesta << std::endl;
}

MensajeDispositivo ServidorParqueadero::parsear_mensaje(const std::string& datos) {
    MensajeDispositivo mensaje;
    
    // Formato esperado: TIPO|PLACA|TIPO_VEHICULO|DISPOSITIVO
    // Ejemplo: ENTRADA|ABC123|carro|CAMARA-01
    
    std::stringstream ss(datos);
    std::string token;
    int campo = 0;
    
    while (std::getline(ss, token, '|')) {
        switch (campo) {
            case 0: mensaje.tipo = token; break;
            case 1: mensaje.placa = token; break;
            case 2: mensaje.tipo_vehiculo = token; break;
            case 3: mensaje.dispositivo = token; break;
        }
        campo++;
    }
    
    return mensaje;
}

std::string ServidorParqueadero::procesar_comando(const MensajeDispositivo& mensaje) {
    std::string resultado;
    bool exito = false;
    
    if (mensaje.tipo == "ENTRADA") {
        resultado = parqueadero->registrar_entrada(mensaje.placa, mensaje.tipo_vehiculo);
        exito = resultado.find("OK") != std::string::npos;
        
        std::cout << "🚗 ENTRADA detectada - " << mensaje.placa 
                  << " (" << mensaje.tipo_vehiculo << ") desde " 
                  << mensaje.dispositivo << std::endl;
    }
    else if (mensaje.tipo == "SALIDA") {
        resultado = parqueadero->registrar_salida(mensaje.placa);
        exito = resultado.find("OK") != std::string::npos;
        
        std::cout << "🚙 SALIDA detectada - " << mensaje.placa 
                  << " desde " << mensaje.dispositivo << std::endl;
    }
    else {
        resultado = "ERROR: Tipo de operación desconocido";
    }
    
    // Notificar al callback (Python)
    if (evento_callback) {
        evento_callback(mensaje.tipo, mensaje.placa, 
                       mensaje.tipo_vehiculo, exito);
    }
    
    return resultado;
}

void ServidorParqueadero::establecer_callback(EventCallback callback) {
    evento_callback = callback;
}

