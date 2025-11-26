#ifndef SERVIDOR_PARQUEADERO_HPP
#define SERVIDOR_PARQUEADERO_HPP

#include "parqueadero.hpp"
#include "socket_utils.hpp"
#include <string>
#include <functional>

// Estructura para mensajes del protocolo
struct MensajeDispositivo {
    std::string tipo;        // "ENTRADA" o "SALIDA"
    std::string placa;       // Placa del vehículo
    std::string tipo_vehiculo; // "carro" o "moto"
    std::string dispositivo; // ID del dispositivo (ej: "CAMARA-01")
};

// Callback para notificar eventos al Python
typedef std::function<void(const std::string&, const std::string&, const std::string&, bool)> EventCallback;

class ServidorParqueadero {
private:
    Parqueadero* parqueadero;
    int puerto;
    socket_t servidor_socket;
    bool ejecutando;
    EventCallback evento_callback;
    
    // Parsear mensaje del dispositivo
    MensajeDispositivo parsear_mensaje(const std::string& datos);
    
    // Procesar comando
    std::string procesar_comando(const MensajeDispositivo& mensaje);
    
    // Manejar cliente
    void manejar_cliente(socket_t cliente_socket);

public:
    ServidorParqueadero(Parqueadero* p, int puerto = 8080);
    ~ServidorParqueadero();
    
    // Iniciar servidor
    bool iniciar();
    
    // Detener servidor
    void detener();
    
    // Aceptar una conexión (bloquea hasta recibir una)
    bool aceptar_conexion();
    
    // Establecer callback para eventos
    void establecer_callback(EventCallback callback);
    
    // Estado del servidor
    bool esta_ejecutando() const { return ejecutando; }
};

#endif





