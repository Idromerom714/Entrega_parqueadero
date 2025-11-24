#include "parqueadero.hpp"
#include <sstream>
#include <iomanip>
#include <cmath>

Parqueadero::Parqueadero(int cap_carros, int cap_motos, 
                         double tarifa_carro, double tarifa_moto)
    : capacidad_carros(cap_carros), 
      capacidad_motos(cap_motos),
      tarifa_hora_carro(tarifa_carro),
      tarifa_hora_moto(tarifa_moto) {
    
    espacios_carros.resize(cap_carros, false);
    espacios_motos.resize(cap_motos, false);
}

std::string Parqueadero::registrar_entrada(const std::string& placa, const std::string& tipo) {
    if (vehiculo_presente(placa)) {
        return "ERROR: El vehículo con placa " + placa + " ya está en el parqueadero";
    }
    
    int espacio = asignar_espacio(tipo);
    if (espacio == -1) {
        return "ERROR: No hay espacios disponibles para " + tipo;
    }
    
    Vehiculo v;
    v.placa = placa;
    v.tipo = tipo;
    v.hora_entrada = time(nullptr);
    v.espacio = espacio;
    
    vehiculos_activos[placa] = v;
    
    std::stringstream ss;
    ss << "OK: Vehículo " << placa << " registrado en espacio " << espacio;
    return ss.str();
}

std::string Parqueadero::registrar_salida(const std::string& placa) {
    if (!vehiculo_presente(placa)) {
        return "ERROR: El vehículo con placa " + placa + " no está en el parqueadero";
    }
    
    Vehiculo v = vehiculos_activos[placa];
    double tarifa = calcular_tarifa(placa);
    
    liberar_espacio(v.tipo, v.espacio);
    vehiculos_activos.erase(placa);
    
    std::stringstream ss;
    ss << "OK: Vehículo " << placa << " retirado. Tarifa: $" 
       << std::fixed << std::setprecision(0) << tarifa;
    return ss.str();
}

bool Parqueadero::vehiculo_presente(const std::string& placa) const {
    return vehiculos_activos.find(placa) != vehiculos_activos.end();
}

int Parqueadero::espacios_disponibles_carros() const {
    int count = 0;
    for (bool ocupado : espacios_carros) {
        if (!ocupado) count++;
    }
    return count;
}

int Parqueadero::espacios_disponibles_motos() const {
    int count = 0;
    for (bool ocupado : espacios_motos) {
        if (!ocupado) count++;
    }
    return count;
}

std::vector<std::string> Parqueadero::listar_vehiculos() const {
    std::vector<std::string> lista;
    for (const auto& par : vehiculos_activos) {
        lista.push_back(par.first);
    }
    return lista;
}

std::string Parqueadero::info_vehiculo(const std::string& placa) const {
    if (!vehiculo_presente(placa)) {
        return "ERROR: Vehículo no encontrado";
    }
    
    const Vehiculo& v = vehiculos_activos.at(placa);
    double tarifa = calcular_tarifa(placa);
    
    char buffer[80];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", localtime(&v.hora_entrada));
    
    std::stringstream ss;
    ss << "Placa: " << v.placa << "\n"
       << "Tipo: " << v.tipo << "\n"
       << "Espacio: " << v.espacio << "\n"
       << "Entrada: " << buffer << "\n"
       << "Tarifa actual: $" << std::fixed << std::setprecision(0) << tarifa;
    
    return ss.str();
}

double Parqueadero::calcular_tarifa(const std::string& placa) const {
    if (!vehiculo_presente(placa)) {
        return 0.0;
    }
    
    const Vehiculo& v = vehiculos_activos.at(placa);
    time_t ahora = time(nullptr);
    double horas = calcular_horas(v.hora_entrada, ahora);
    
    double tarifa_hora = (v.tipo == "carro") ? tarifa_hora_carro : tarifa_hora_moto;
    return horas * tarifa_hora;
}

int Parqueadero::asignar_espacio(const std::string& tipo) {
    std::vector<bool>& espacios = (tipo == "carro") ? espacios_carros : espacios_motos;
    
    for (size_t i = 0; i < espacios.size(); i++) {
        if (!espacios[i]) {
            espacios[i] = true;
            return i + 1;
        }
    }
    return -1;
}

void Parqueadero::liberar_espacio(const std::string& tipo, int espacio) {
    std::vector<bool>& espacios = (tipo == "carro") ? espacios_carros : espacios_motos;
    if (espacio > 0 && espacio <= (int)espacios.size()) {
        espacios[espacio - 1] = false;
    }
}

double Parqueadero::calcular_horas(time_t entrada, time_t salida) const {
    double segundos = difftime(salida, entrada);
    double horas = segundos / 3600.0;
    return std::ceil(horas); // Redondear hacia arriba
}

