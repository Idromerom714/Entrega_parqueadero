# Librería C++ - Core del Sistema

La librería C++ es el **corazón del sistema**, implementando toda la lógica crítica de negocio con máximo rendimiento.

## Estructura de Archivos

```
cpp/
├── parqueadero.hpp      # Declaración de clases y métodos
├── parqueadero.cpp      # Implementación de la lógica
└── bindings.cpp         # Bindings para Python (ver wiki separada)
```

## Arquitectura de la Clase

### Diagrama de Clases

```
┌─────────────────────────────────────────┐
│          Parqueadero                    │
├─────────────────────────────────────────┤
│ - capacidad_carros: int                 │
│ - capacidad_motos: int                  │
│ - vehiculos_activos: map<str, Vehiculo> │
│ - espacios_carros: vector<bool>         │
│ - espacios_motos: vector<bool>          │
│ - tarifa_hora_carro: double             │
│ - tarifa_hora_moto: double              │
├─────────────────────────────────────────┤
│ + Parqueadero(int, int, double, double) │
│ + registrar_entrada(str, str): string   │
│ + registrar_salida(str): string         │
│ + vehiculo_presente(str): bool          │
│ + espacios_disponibles_carros(): int    │
│ + espacios_disponibles_motos(): int     │
│ + listar_vehiculos(): vector<string>    │
│ + info_vehiculo(str): string            │
│ + calcular_tarifa(str): double          │
│ - asignar_espacio(str): int             │
│ - liberar_espacio(str, int): void       │
│ - calcular_horas(time_t, time_t): double│
└─────────────────────────────────────────┘
          ▲
          │ usa
          │
┌─────────────────────┐
│     Vehiculo        │
├─────────────────────┤
│ + placa: string     │
│ + tipo: string      │
│ + hora_entrada: time│
│ + espacio: int      │
└─────────────────────┘
```

## Detalles de Implementación

### 1. Gestión de Espacios

Los espacios se manejan con **vectores booleanos** para eficiencia:

```cpp
std::vector<bool> espacios_carros;   // false = libre, true = ocupado
std::vector<bool> espacios_motos;
```

**¿Por qué vector<bool>?**
- Optimización de memoria (1 bit por espacio)
- Acceso O(1) constante
- Fácil conteo de disponibles

**Asignación de espacio:**
```cpp
int Parqueadero::asignar_espacio(const std::string& tipo) {
    std::vector<bool>& espacios = (tipo == "carro") ? 
                                   espacios_carros : espacios_motos;
    
    for (size_t i = 0; i < espacios.size(); i++) {
        if (!espacios[i]) {          // Si está libre
            espacios[i] = true;       // Marcarlo como ocupado
            return i + 1;             // Retornar número (1-indexed)
        }
    }
    return -1;  // No hay espacios
}
```

### 2. Tracking de Vehículos

Se usa un `std::map` para almacenar vehículos activos:

```cpp
std::map<std::string, Vehiculo> vehiculos_activos;
// Key: placa (ej: "ABC123")
// Value: struct Vehiculo con toda la info
```

**Ventajas del map:**
- Búsqueda rápida por placa: O(log n)
- Orden automático alfabético
- No permite duplicados (clave única)

**Estructura Vehiculo:**
```cpp
struct Vehiculo {
    std::string placa;      // "ABC123"
    std::string tipo;       // "carro" o "moto"
    time_t hora_entrada;    // Timestamp Unix
    int espacio;            // Número del espacio asignado
};
```

### 3. Registro de Entrada

**Flujo completo:**

```cpp
std::string Parqueadero::registrar_entrada(const std::string& placa, 
                                           const std::string& tipo) {
    // 1. Validar duplicado
    if (vehiculo_presente(placa)) {
        return "ERROR: El vehículo con placa " + placa + 
               " ya está en el parqueadero";
    }
    
    // 2. Intentar asignar espacio
    int espacio = asignar_espacio(tipo);
    if (espacio == -1) {
        return "ERROR: No hay espacios disponibles para " + tipo;
    }
    
    // 3. Crear registro
    Vehiculo v;
    v.placa = placa;
    v.tipo = tipo;
    v.hora_entrada = time(nullptr);  // Timestamp actual
    v.espacio = espacio;
    
    // 4. Guardar en mapa
    vehiculos_activos[placa] = v;
    
    // 5. Retornar confirmación
    std::stringstream ss;
    ss << "OK: Vehículo " << placa << " registrado en espacio " << espacio;
    return ss.str();
}
```

**Complejidad:** O(log n) debido a la inserción en map

### 4. Registro de Salida

```cpp
std::string Parqueadero::registrar_salida(const std::string& placa) {
    // 1. Verificar que existe
    if (!vehiculo_presente(placa)) {
        return "ERROR: El vehículo con placa " + placa + 
               " no está en el parqueadero";
    }
    
    // 2. Obtener datos del vehículo
    Vehiculo v = vehiculos_activos[placa];
    
    // 3. Calcular tarifa
    double tarifa = calcular_tarifa(placa);
    
    // 4. Liberar espacio
    liberar_espacio(v.tipo, v.espacio);
    
    // 5. Eliminar del mapa
    vehiculos_activos.erase(placa);
    
    // 6. Retornar resultado con tarifa
    std::stringstream ss;
    ss << "OK: Vehículo " << placa << " retirado. Tarifa: $" 
       << std::fixed << std::setprecision(0) << tarifa;
    return ss.str();
}
```

### 5. Cálculo de Tarifas

**Algoritmo:**

1. Obtener timestamp de entrada
2. Obtener timestamp actual
3. Calcular diferencia en segundos
4. Convertir a horas
5. **Redondear hacia arriba** (ceil)
6. Multiplicar por tarifa según tipo

```cpp
double Parqueadero::calcular_tarifa(const std::string& placa) const {
    const Vehiculo& v = vehiculos_activos.at(placa);
    
    // Obtener tiempo actual
    time_t ahora = time(nullptr);
    
    // Calcular horas
    double horas = calcular_horas(v.hora_entrada, ahora);
    
    // Aplicar tarifa según tipo
    double tarifa_hora = (v.tipo == "carro") ? 
                         tarifa_hora_carro : tarifa_hora_moto;
    
    return horas * tarifa_hora;
}

double Parqueadero::calcular_horas(time_t entrada, time_t salida) const {
    double segundos = difftime(salida, entrada);
    double horas = segundos / 3600.0;
    return std::ceil(horas);  // IMPORTANTE: Redondear arriba
}
```

**Ejemplo:**
- Entrada: 10:00 AM
- Salida: 10:30 AM
- Diferencia: 30 minutos = 0.5 horas
- `ceil(0.5)` = 1 hora
- **Se cobra 1 hora completa**

### 6. Consultas de Estado

**Espacios disponibles:**
```cpp
int Parqueadero::espacios_disponibles_carros() const {
    int count = 0;
    for (bool ocupado : espacios_carros) {
        if (!ocupado) count++;
    }
    return count;
}
```
**Complejidad:** O(n) donde n = capacidad

**Listar vehículos:**
```cpp
std::vector<std::string> Parqueadero::listar_vehiculos() const {
    std::vector<std::string> lista;
    for (const auto& par : vehiculos_activos) {
        lista.push_back(par.first);  // Agregar solo las placas
    }
    return lista;
}
```
**Complejidad:** O(n) donde n = vehículos activos

**Info detallada:**
```cpp
std::string Parqueadero::info_vehiculo(const std::string& placa) const {
    if (!vehiculo_presente(placa)) {
        return "ERROR: Vehículo no encontrado";
    }
    
    const Vehiculo& v = vehiculos_activos.at(placa);
    double tarifa = calcular_tarifa(placa);
    
    // Formatear fecha
    char buffer[80];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", 
             localtime(&v.hora_entrada));
    
    // Construir string con toda la info
    std::stringstream ss;
    ss << "Placa: " << v.placa << "\n"
       << "Tipo: " << v.tipo << "\n"
       << "Espacio: " << v.espacio << "\n"
       << "Entrada: " << buffer << "\n"
       << "Tarifa actual: $" << std::fixed << std::setprecision(0) << tarifa;
    
    return ss.str();
}
```

## Optimizaciones

### 1. Paso por Referencia Constante

```cpp
// ❌ Ineficiente (copia el string)
bool vehiculo_presente(std::string placa);

// ✅ Eficiente (referencia, sin copia)
bool vehiculo_presente(const std::string& placa) const;
```

### 2. Uso de std::map

- Búsquedas O(log n) vs O(n) de vector
- Inserciones ordenadas automáticas
- No permite duplicados por diseño

### 3. Vector de Booleanos

```cpp
std::vector<bool> espacios;  // 1 bit por espacio
```
vs
```cpp
std::vector<int> espacios;   // 32/64 bits por espacio
```

Ahorro de memoria de **32x a 64x**

## Validaciones Implementadas

| Operación | Validaciones |
|-----------|--------------|
| Entrada | • Placa no duplicada<br>• Espacios disponibles<br>• Tipo válido ("carro"/"moto") |
| Salida | • Vehículo presente<br>• Placa válida |
| Consultas | • Vehículo existe antes de calcular tarifa |

## Formato de Respuestas

Todas las operaciones retornan strings con prefijo:

**Éxito:**
```
"OK: Mensaje de confirmación"
```

**Error:**
```
"ERROR: Mensaje descriptivo del error"
```

Esto facilita el parsing en Python:
```python
resultado = parqueadero.registrar_entrada("ABC123", "carro")
if resultado.startswith("OK"):
    # Operación exitosa
else:
    # Manejar error
```

## Complejidad Temporal

| Operación | Complejidad |
|-----------|-------------|
| `registrar_entrada` | O(log n) |
| `registrar_salida` | O(log n) |
| `vehiculo_presente` | O(log n) |
| `espacios_disponibles` | O(n) |
| `listar_vehiculos` | O(n) |
| `calcular_tarifa` | O(log n) |

Donde n = número de vehículos activos

## Compilación

El código C++ se compila a una librería compartida:

**Linux/Mac:**
```bash
g++ -O3 -Wall -shared -std=c++11 -fPIC \
    cpp/parqueadero.cpp cpp/bindings.cpp \
    -o parqueadero_cpp.so
```

**Windows:**
```bash
g++ -O3 -Wall -shared -std=c++11 \
    cpp/parqueadero.cpp cpp/bindings.cpp \
    -o parqueadero_cpp.pyd
```

Flags importantes:
- `-O3`: Optimización máxima
- `-std=c++11`: Usar C++11 (mínimo)
- `-fPIC`: Position Independent Code (Linux/Mac)
- `-shared`: Crear librería compartida

## Testing desde C++

Puedes crear un archivo de prueba:

```cpp
// test_parqueadero.cpp
#include "parqueadero.hpp"
#include <iostream>

int main() {
    Parqueadero p(10, 20, 3000, 2000);
    
    std::cout << p.registrar_entrada("ABC123", "carro") << std::endl;
    std::cout << "Espacios: " << p.espacios_disponibles_carros() << std::endl;
    std::cout << p.registrar_salida("ABC123") << std::endl;
    
    return 0;
}
```

Compilar y ejecutar:
```bash
g++ -std=c++11 test_parqueadero.cpp cpp/parqueadero.cpp -o test
./test
```

## Ver También

- [Bindings pybind11](Pybind11-Bindings) - Cómo se expone a Python
- [API Reference](API-Reference) - Documentación completa de métodos
- [Architecture Overview](Architecture-Overview) - Visión general del sistema

