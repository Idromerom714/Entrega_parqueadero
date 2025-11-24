#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include "parqueadero.hpp"
#include "servidor_parqueadero.hpp"
#include <pybind11/functional.h>

namespace py = pybind11;

PYBIND11_MODULE(parqueadero_cpp, m) {
    m.doc() = "Sistema de gestión de parqueadero en C++";
    
    py::class_<Parqueadero>(m, "Parqueadero")
        .def(py::init<int, int, double, double>(),
             py::arg("cap_carros"),
             py::arg("cap_motos"),
             py::arg("tarifa_carro") = 3000.0,
             py::arg("tarifa_moto") = 2000.0,
             "Constructor del parqueadero")
        
        .def("registrar_entrada", &Parqueadero::registrar_entrada,
             py::arg("placa"), py::arg("tipo"),
             "Registra la entrada de un vehículo")
        
        .def("registrar_salida", &Parqueadero::registrar_salida,
             py::arg("placa"),
             "Registra la salida de un vehículo y calcula tarifa")
        
        .def("vehiculo_presente", &Parqueadero::vehiculo_presente,
             py::arg("placa"),
             "Verifica si un vehículo está en el parqueadero")
        
        .def("espacios_disponibles_carros", &Parqueadero::espacios_disponibles_carros,
             "Retorna el número de espacios disponibles para carros")
        
        .def("espacios_disponibles_motos", &Parqueadero::espacios_disponibles_motos,
             "Retorna el número de espacios disponibles para motos")
        
        .def("listar_vehiculos", &Parqueadero::listar_vehiculos,
             "Lista todas las placas de vehículos presentes")
        
        .def("info_vehiculo", &Parqueadero::info_vehiculo,
             py::arg("placa"),
             "Obtiene información detallada de un vehículo")
        
        .def("calcular_tarifa", &Parqueadero::calcular_tarifa,
             py::arg("placa"),
             "Calcula la tarifa actual de un vehículo");

    // Binding para ServidorParqueadero
    py::class_<ServidorParqueadero>(m, "ServidorParqueadero")
        .def(py::init<Parqueadero*, int>(),
             py::arg("parqueadero"), py::arg("puerto") = 8080)
        .def("iniciar", &ServidorParqueadero::iniciar,
             "Inicia el servidor TCP")
        .def("detener", &ServidorParqueadero::detener,
             "Detiene el servidor TCP")
        .def("aceptar_conexion", &ServidorParqueadero::aceptar_conexion,
             "Acepta una conexión entrante (bloqueante)")
        .def("esta_ejecutando", &ServidorParqueadero::esta_ejecutando,
             "Retorna True si el servidor está ejecutando")
        .def("establecer_callback", [](ServidorParqueadero &s, py::function cb){
            // Guardar el callback en una lambda que adquiere el GIL
            s.establecer_callback([cb](const std::string& tipo,
                                      const std::string& placa,
                                      const std::string& tipo_vehiculo,
                                      bool exito){
                py::gil_scoped_acquire acquire;
                try {
                    cb(tipo, placa, tipo_vehiculo, exito);
                } catch (const std::exception &e) {
                    // Evitar que excepciones en Python rompan el servidor C++
                    py::print("Exception in callback:", e.what());
                }
            });
        }, "Establece un callback Python para eventos (tipo, placa, tipo_vehiculo, exito)");
}
