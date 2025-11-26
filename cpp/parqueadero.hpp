#ifndef PARQUEADERO_HPP
#define PARQUEADERO_HPP

#include <string>
#include <vector>
#include <map>
#include <ctime>

struct Vehiculo {
    std::string placa;
    std::string tipo; // "carro", "moto"
    time_t hora_entrada;
    int espacio;
};

class Parqueadero {
private:
    int capacidad_carros;
    int capacidad_motos;
    std::map<std::string, Vehiculo> vehiculos_activos; // placa -> vehiculo
    std::vector<bool> espacios_carros;
    std::vector<bool> espacios_motos;
    
    double tarifa_hora_carro;
    double tarifa_hora_moto;

public:
    Parqueadero(int cap_carros, int cap_motos, 
                double tarifa_carro = 3000.0, double tarifa_moto = 2000.0);
    
    // Operaciones principales
    std::string registrar_entrada(const std::string& placa, const std::string& tipo);
    std::string registrar_salida(const std::string& placa);
    
    // Consultas
    bool vehiculo_presente(const std::string& placa) const;
    int espacios_disponibles_carros() const;
    int espacios_disponibles_motos() const;
    std::vector<std::string> listar_vehiculos() const;
    std::string info_vehiculo(const std::string& placa) const;
    
    // Cálculo de tarifa
    double calcular_tarifa(const std::string& placa) const;

private:
    int asignar_espacio(const std::string& tipo);
    void liberar_espacio(const std::string& tipo, int espacio);
    double calcular_horas(time_t entrada, time_t salida) const;
};

#endif





