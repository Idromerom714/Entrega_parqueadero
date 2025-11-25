#include "cpp/socket_utils.hpp"
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <cstdlib>
#include <ctime>

#ifdef _WIN32
    #include <windows.h>
    #define SLEEP(ms) Sleep(ms)
#else
    #include <unistd.h>
    #define SLEEP(ms) usleep((ms) * 1000)
#endif

class DispositivoSimulador {
private:
    std::string id_dispositivo;
    std::string servidor_ip;
    int servidor_puerto;
    std::vector<std::string> placas_disponibles;
    std::vector<std::string> vehiculos_dentro;  // ← NUEVO: tracking local
    
    std::string generar_placa_aleatoria() {
        if (placas_disponibles.empty()) {
            // Generar placa aleatoria ABC123
            std::string placa = "";
            for (int i = 0; i < 3; i++) {
                placa += (char)('A' + rand() % 26);
            }
            for (int i = 0; i < 3; i++) {
                placa += (char)('0' + rand() % 10);
            }
            return placa;
        }
        return placas_disponibles[rand() % placas_disponibles.size()];
    }
    
    std::string generar_placa_para_entrada() {
        // ← NUEVO: Solo placas que NO están dentro
        std::vector<std::string> disponibles;
        for (const auto& placa : placas_disponibles) {
            bool esta_dentro = false;
            for (const auto& v : vehiculos_dentro) {
                if (v == placa) {
                    esta_dentro = true;
                    break;
                }
            }
            if (!esta_dentro) {
                disponibles.push_back(placa);
            }
        }
        
        if (disponibles.empty()) {
            return "";  // No hay placas disponibles
        }
        return disponibles[rand() % disponibles.size()];
    }
    
    std::string generar_placa_para_salida() {
        // ← NUEVO: Solo placas que SÍ están dentro
        if (vehiculos_dentro.empty()) {
            return "";
        }
        return vehiculos_dentro[rand() % vehiculos_dentro.size()];
    }
    
    std::string generar_tipo_aleatorio() {
        return (rand() % 2 == 0) ? "carro" : "moto";
    }
    
    bool enviar_evento(const std::string& tipo, const std::string& placa, 
                       const std::string& tipo_vehiculo) {
        if (!inicializar_sockets()) {
            std::cerr << "❌ Error al inicializar sockets" << std::endl;
            return false;
        }
        
        // Crear socket
        socket_t sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock == INVALID_SOCKET) {
            std::cerr << "❌ Error al crear socket: " << obtener_error_socket() << std::endl;
            limpiar_sockets();
            return false;
        }
        
        // Configurar dirección del servidor
        struct sockaddr_in serv_addr;
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(servidor_puerto);
        
        // Convertir IP
#ifdef _WIN32
        serv_addr.sin_addr.s_addr = inet_addr(servidor_ip.c_str());
#else
        if (inet_pton(AF_INET, servidor_ip.c_str(), &serv_addr.sin_addr) <= 0) {
            std::cerr << "❌ Dirección IP inválida" << std::endl;
            CLOSE_SOCKET(sock);
            limpiar_sockets();
            return false;
        }
#endif
        
        // Conectar al servidor
        if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == SOCKET_ERROR) {
            std::cerr << "❌ Error al conectar: " << obtener_error_socket() << std::endl;
            CLOSE_SOCKET(sock);
            limpiar_sockets();
            return false;
        }
        
        std::cout << "✅ Conectado al servidor" << std::endl;
        
        // Construir mensaje: TIPO|PLACA|TIPO_VEHICULO|DISPOSITIVO
        std::stringstream ss;
        ss << tipo << "|" << placa << "|" << tipo_vehiculo << "|" << id_dispositivo;
        std::string mensaje = ss.str();
        
        // Enviar mensaje
        send(sock, mensaje.c_str(), mensaje.length(), 0);
        std::cout << "📤 Enviado: " << mensaje << std::endl;
        
        // Recibir respuesta
        char buffer[1024] = {0};
        int bytes = recv(sock, buffer, sizeof(buffer) - 1, 0);
        
        bool exito = false;
        if (bytes > 0) {
            buffer[bytes] = '\0';
            std::cout << "📥 Respuesta: " << buffer << std::endl;
            
            // Verificar si fue exitoso
            std::string respuesta(buffer);
            exito = (respuesta.find("OK") != std::string::npos);
            
            // ← NUEVO: Actualizar tracking local
            if (exito) {
                if (tipo == "ENTRADA") {
                    vehiculos_dentro.push_back(placa);
                } else if (tipo == "SALIDA") {
                    // Eliminar de la lista
                    for (auto it = vehiculos_dentro.begin(); it != vehiculos_dentro.end(); ++it) {
                        if (*it == placa) {
                            vehiculos_dentro.erase(it);
                            break;
                        }
                    }
                }
            }
        }
        
        CLOSE_SOCKET(sock);
        limpiar_sockets();
        
        return exito;
    }

public:
    DispositivoSimulador(const std::string& id, const std::string& ip = "127.0.0.1", 
                         int puerto = 8080)
        : id_dispositivo(id), servidor_ip(ip), servidor_puerto(puerto) {
        
        // Placas predefinidas para simulación
        placas_disponibles = {
            "ABC123", "DEF456", "GHI789", "JKL012", "MNO345",
            "PQR678", "STU901", "VWX234", "YZA567", "BCD890"
        };
        
        srand(time(nullptr));
    }
    
    void simular_entrada() {
        std::string placa = generar_placa_para_entrada();  // ← Usa la nueva función
        
        if (placa.empty()) {
            std::cout << "\n⚠️  [" << id_dispositivo << "] No hay placas disponibles para entrada" << std::endl;
            std::cout << "   (Todos los vehículos ya están dentro)" << std::endl;
            return;
        }
        
        std::string tipo = generar_tipo_aleatorio();
        
        std::cout << "\n🚗 [" << id_dispositivo << "] Detectando ENTRADA..." << std::endl;
        std::cout << "   Placa: " << placa << " | Tipo: " << tipo << std::endl;
        
        enviar_evento("ENTRADA", placa, tipo);
    }
    
    void simular_salida() {
        std::string placa = generar_placa_para_salida();  // ← Usa la nueva función
        
        if (placa.empty()) {
            std::cout << "\n⚠️  [" << id_dispositivo << "] No hay vehículos para sacar" << std::endl;
            std::cout << "   (El parqueadero está vacío según este dispositivo)" << std::endl;
            return;
        }
        
        std::cout << "\n🚙 [" << id_dispositivo << "] Detectando SALIDA..." << std::endl;
        std::cout << "   Placa: " << placa << std::endl;
        
        enviar_evento("SALIDA", placa, "");
    }
    
    void simular_trafico(int num_eventos = 10, int delay_ms = 2000) {
        std::cout << "🤖 Iniciando simulación de tráfico..." << std::endl;
        std::cout << "   Dispositivo: " << id_dispositivo << std::endl;
        std::cout << "   Servidor: " << servidor_ip << ":" << servidor_puerto << std::endl;
        std::cout << "   Eventos: " << num_eventos << std::endl;
        std::cout << "   Delay: " << delay_ms << "ms\n" << std::endl;
        
        for (int i = 0; i < num_eventos; i++) {
            std::cout << "--- Evento " << (i + 1) << "/" << num_eventos << " ---" << std::endl;
            std::cout << "📊 Estado local: " << vehiculos_dentro.size() << " vehículos dentro" << std::endl;
            
            // ← NUEVA LÓGICA ADAPTATIVA
            bool hacer_entrada;
            
            if (vehiculos_dentro.empty()) {
                // Si está vacío, solo entrada
                hacer_entrada = true;
            } else if (vehiculos_dentro.size() >= placas_disponibles.size()) {
                // Si todos los vehículos están dentro, solo salida
                hacer_entrada = false;
            } else if (vehiculos_dentro.size() < 3) {
                // Pocos vehículos, favorecer entradas (70%)
                hacer_entrada = (rand() % 100 < 70);
            } else {
                // Balance normal (50-50)
                hacer_entrada = (rand() % 2 == 0);
            }
            
            if (hacer_entrada) {
                simular_entrada();
            } else {
                simular_salida();
            }
            
            if (i < num_eventos - 1) {
                std::cout << "⏳ Esperando " << (delay_ms / 1000) << " segundos..." << std::endl;
                SLEEP(delay_ms);
            }
        }
        
        std::cout << "\n✅ Simulación completada" << std::endl;
        std::cout << "📊 Estado final: " << vehiculos_dentro.size() << " vehículos dentro" << std::endl;
    }
    
    void modo_interactivo() {
        std::cout << "\n🎮 Modo Interactivo - Simulador de Dispositivo" << std::endl;
        std::cout << "   Dispositivo: " << id_dispositivo << std::endl;
        std::cout << "   Servidor: " << servidor_ip << ":" << servidor_puerto << std::endl;
        
        while (true) {
            std::cout << "\n┌─────────────────────────────┐" << std::endl;
            std::cout << "│  1. Simular ENTRADA         │" << std::endl;
            std::cout << "│  2. Simular SALIDA          │" << std::endl;
            std::cout << "│  3. Tráfico automático      │" << std::endl;
            std::cout << "│  4. Salir                   │" << std::endl;
            std::cout << "└─────────────────────────────┘" << std::endl;
            std::cout << "Opción: ";
            
            int opcion;
            std::cin >> opcion;
            
            switch (opcion) {
                case 1:
                    simular_entrada();
                    break;
                case 2:
                    simular_salida();
                    break;
                case 3: {
                    int num_eventos;
                    std::cout << "Número de eventos: ";
                    std::cin >> num_eventos;
                    simular_trafico(num_eventos, 2000);
                    break;
                }
                case 4:
                    std::cout << "👋 Saliendo..." << std::endl;
                    return;
                default:
                    std::cout << "❌ Opción inválida" << std::endl;
            }
        }
    }
};

int main(int argc, char* argv[]) {
    std::string id_dispositivo = "CAMARA-01";
    std::string servidor_ip = "127.0.0.1";
    int servidor_puerto = 8080;
    
    // Parsear argumentos
    if (argc > 1) id_dispositivo = argv[1];
    if (argc > 2) servidor_ip = argv[2];
    if (argc > 3) servidor_puerto = std::atoi(argv[3]);
    
    DispositivoSimulador dispositivo(id_dispositivo, servidor_ip, servidor_puerto);
    
    std::cout << "╔════════════════════════════════════════╗" << std::endl;
    std::cout << "║  Simulador de Dispositivo IoT          ║" << std::endl;
    std::cout << "║  Sistema de Parqueadero                ║" << std::endl;
    std::cout << "╚════════════════════════════════════════╝" << std::endl;
    
    // Modo automático o interactivo
    if (argc > 4 && std::string(argv[4]) == "auto") {
        int num_eventos = (argc > 5) ? std::atoi(argv[5]) : 10;
        dispositivo.simular_trafico(num_eventos, 2000);
    } else {
        dispositivo.modo_interactivo();
    }
    
    return 0;
}
