# 🅿️ Sistema de Gestión de Parqueadero

[![Python 3.7+](https://img.shields.io/badge/python-3.7+-blue.svg)](https://www.python.org/downloads/)
[![C++11](https://img.shields.io/badge/C++-11-blue.svg)](https://en.cppreference.com/w/cpp/11)

Sistema completo de gestión de parqueadero que combina la **eficiencia de C++** para la lógica de negocio con la **flexibilidad de Python** para la interfaz web y simulador automatizado. Posibilidad de añadir autenticación de usuarios, persistencia de datos con SQLite, reportes en tiempo real.

## 👥 Autores
- **Juan** - *Core/Creación de libreria C++* - [Idromerom714](https://github.com/Idromerom714)
- **Estela** - *Backend en python/consumo de la libreria por el simulador* - [Idromerom714](https://github.com/Idromerom714)
- **Iván** - *Conexión Core-Backend con pybind11* - [Idromerom714](https://github.com/Idromerom714)

## 📑 Tabla de Contenidos

- [Características](#-características)
- [Arquitectura del Sistema](#-arquitectura-del-sistema)
- [Instalación](#-instalación)
- [Componentes Principales](#-componentes-principales)
  - [Librería C++](#1-librería-c-core-del-sistema)
  - [Bindings con pybind11](#2-bindings-con-pybind11)
  - [Aplicación Web Flask](#3-aplicación-web-flask)
  - [Simulador Automatizado](#4-simulador-automatizado)
- [Uso](#-uso)
- [API Reference](#-api-reference)
- [Configuración](#-configuración)
- [Contribuir](#-contribuir)
- [Licencia](#-licencia)

## ✨ Características

### 🚀 Core en C++
- ✅ **Alto rendimiento** en operaciones críticas
- ✅ Gestión eficiente de espacios (carros y motos)
- ✅ Cálculo automático de tarifas por tiempo
- ✅ Validaciones y control de estado en tiempo real

### 🐍 Backend Python
- ✅ **Flask** para API RESTful
- ✅ **SQLite** para persistencia de datos
- ✅ Sistema de autenticación con roles (Admin/Operador)
- ✅ Historial completo de transacciones
- ✅ Reportes y estadísticas

### 🌐 Interfaz Web (extra)
- ✅ Dashboard responsive y moderno
- ✅ Actualización en tiempo real
- ✅ Gestión de usuarios (solo admin)
- ✅ Reportes con filtros por fecha

### 🤖 Simulador
- ✅ Generación automática de tráfico
- ✅ Lógica adaptativa según ocupación
- ✅ Estadísticas detalladas
- ✅ Ideal para testing y demos

## 🏗️ Arquitectura del Sistema

```
┌─────────────────────────────────────────────────────────┐
│                     INTERFAZ WEB                        │
│              (HTML/CSS/JavaScript)                      │
└────────────────────┬────────────────────────────────────┘
                     │
┌────────────────────▼────────────────────────────────────┐
│                  FLASK API                              │
│         (app.py - Python Backend)                       │
│                                                         │
│  ┌─────────────┐              ┌──────────────┐          │
│  │  Database   │              │   Sessions   │          │
│  │  (SQLite)   │              │   & Auth     │          │
│  └─────────────┘              └──────────────┘          │
└────────────────────┬────────────────────────────────────┘
                     │
┌────────────────────▼────────────────────────────────────┐
│              PYTHON BINDINGS                            │
│               (pybind11)                                │
└────────────────────┬────────────────────────────────────┘
                     │
┌────────────────────▼────────────────────────────────────┐
│              LIBRERÍA C++                               │
│         (Lógica Core del Negocio)                       │
│                                                         │
│  • Gestión de espacios                                  │
│  • Cálculo de tarifas                                   │
│  • Validaciones de entrada/salida                       │
│  • Control de estado en memoria                         │
└─────────────────────────────────────────────────────────┘
```

## 📥 Instalación

### Requisitos Previos

- **Python 3.7+**
- **Compilador C++** (g++, clang, o MSVC)
- **pip** (gestor de paquetes de Python)
- **make** (opcional, pero recomendado)

### Paso 1: Clonar el repositorio

```bash
git clone https://github.com/Idromerom714/parqueadero_.git
cd parqueadero_
```

### Paso 2: Instalar dependencias de Python

```bash
pip install -r requirements.txt
```

O manualmente:
```bash
pip install flask pybind11 werkzeug
```

### Paso 3: Compilar el módulo C++

**Opción A - Usando Makefile (Recomendado):**
```bash
make
```

**Opción B - Usando setup.py:**
```bash
python setup.py build_ext --inplace
```

**Opción C - Compilación manual:**
```bash
# Linux/Mac
g++ -O3 -Wall -shared -std=c++11 -fPIC \
  $(python3 -m pybind11 --includes) \
  -Icpp cpp/parqueadero.cpp cpp/bindings.cpp \
  -o parqueadero_cpp$(python3-config --extension-suffix)

# Windows (MinGW)
g++ -O3 -Wall -shared -std=c++11 \
  -I"%PYTHON_PATH%\include" \
  -I"%PYTHON_PATH%\Lib\site-packages\pybind11\include" \
  -Icpp cpp/parqueadero.cpp cpp/bindings.cpp \
  -o parqueadero_cpp.pyd
```

### Paso 4: Verificar instalación

```bash
make test
# o
python -c "import parqueadero_cpp; print('✓ Módulo compilado correctamente')"
```

## 🔧 Componentes Principales

### 1. Librería C++ (Core del Sistema)

La lógica central del parqueadero está implementada en C++ para máximo rendimiento.

#### Archivo: `cpp/parqueadero.hpp`

Define la clase `Parqueadero` con todos sus métodos y atributos:

```cpp
class Parqueadero {
private:
    int capacidad_carros;
    int capacidad_motos;
    std::map<std::string, Vehiculo> vehiculos_activos;
    std::vector<bool> espacios_carros;
    std::vector<bool> espacios_motos;
    double tarifa_hora_carro;
    double tarifa_hora_moto;

public:
    Parqueadero(int cap_carros, int cap_motos, 
                double tarifa_carro, double tarifa_moto);
    
    std::string registrar_entrada(const std::string& placa, 
                                   const std::string& tipo);
    std::string registrar_salida(const std::string& placa);
    bool vehiculo_presente(const std::string& placa) const;
    // ... más métodos
};
```

#### Archivo: `cpp/parqueadero.cpp`

Implementación de toda la lógica:

**Características clave:**
- **Gestión de espacios:** Usa vectores booleanos para tracking eficiente
- **Cálculo de tarifas:** Basado en diferencia de tiempo (redondea hacia arriba)
- **Validaciones:** Previene duplicados, espacios llenos, placas inexistentes
- **Retorno de mensajes:** Formato "OK:" o "ERROR:" para parsing fácil

**Ejemplo de lógica interna:**

```cpp
std::string Parqueadero::registrar_entrada(const std::string& placa, 
                                           const std::string& tipo) {
    // 1. Validar que no esté duplicado
    if (vehiculo_presente(placa)) {
        return "ERROR: El vehículo ya está en el parqueadero";
    }
    
    // 2. Asignar espacio disponible
    int espacio = asignar_espacio(tipo);
    if (espacio == -1) {
        return "ERROR: No hay espacios disponibles";
    }
    
    // 3. Crear registro del vehículo
    Vehiculo v;
    v.placa = placa;
    v.tipo = tipo;
    v.hora_entrada = time(nullptr);  // Timestamp actual
    v.espacio = espacio;
    
    // 4. Guardar en mapa activo
    vehiculos_activos[placa] = v;
    
    return "OK: Vehículo registrado en espacio " + std::to_string(espacio);
}
```

**Cálculo de tarifa:**
```cpp
double Parqueadero::calcular_tarifa(const std::string& placa) const {
    const Vehiculo& v = vehiculos_activos.at(placa);
    time_t ahora = time(nullptr);
    
    // Calcular horas transcurridas
    double segundos = difftime(ahora, v.hora_entrada);
    double horas = std::ceil(segundos / 3600.0);  // Redondear arriba
    
    // Aplicar tarifa según tipo
    double tarifa_hora = (v.tipo == "carro") ? 
                         tarifa_hora_carro : tarifa_hora_moto;
    
    return horas * tarifa_hora;
}
```

### 2. Bindings con pybind11

#### Archivo: `cpp/bindings.cpp`

Este archivo es el **puente entre C++ y Python**. Usa pybind11 para exponer la clase C++ a Python.

```cpp
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>  // Para soporte de std::vector, std::string
#include "parqueadero.hpp"

namespace py = pybind11;

PYBIND11_MODULE(parqueadero_cpp, m) {
    m.doc() = "Sistema de gestión de parqueadero en C++";
    
    // Exponer la clase Parqueadero
    py::class_<Parqueadero>(m, "Parqueadero")
        // Constructor
        .def(py::init<int, int, double, double>(),
             py::arg("cap_carros"),
             py::arg("cap_motos"),
             py::arg("tarifa_carro") = 3000.0,
             py::arg("tarifa_moto") = 2000.0,
             "Constructor del parqueadero")
        
        // Métodos
        .def("registrar_entrada", &Parqueadero::registrar_entrada)
        .def("registrar_salida", &Parqueadero::registrar_salida)
        .def("vehiculo_presente", &Parqueadero::vehiculo_presente)
        .def("espacios_disponibles_carros", 
             &Parqueadero::espacios_disponibles_carros)
        // ... más métodos
}
```

**¿Qué hace pybind11?**

1. **Convierte tipos automáticamente:**
   - `std::string` ↔ `str` de Python
   - `int` ↔ `int` de Python
   - `std::vector<std::string>` ↔ `list` de Python
   - `bool` ↔ `bool` de Python

2. **Maneja memoria:** Gestiona automáticamente la creación/destrucción de objetos C++

3. **Genera documentación:** Los docstrings están disponibles en Python

4. **Manejo de excepciones:** Convierte excepciones de C++ a Python

**Uso desde Python:**

```python
import parqueadero_cpp

# Crear instancia (llama al constructor de C++)
p = parqueadero_cpp.Parqueadero(20, 30, 3000.0, 2000.0)

# Llamar métodos (ejecutan código C++)
resultado = p.registrar_entrada("ABC123", "carro")
espacios = p.espacios_disponibles_carros()  # int
vehiculos = p.listar_vehiculos()  # list[str]
```

### 3. Aplicación Web Flask

#### Archivo: `app.py`

Backend completo con Flask que orquesta todo el sistema.

**Componentes principales:**

```python
# 1. Crear instancia del parqueadero C++
parqueadero = parqueadero_cpp.Parqueadero(20, 30, 3000.0, 2000.0)

# 2. Inicializar base de datos
db = Database()

# 3. Configurar Flask con sesiones
app = Flask(__name__)
app.secret_key = 'clave-secreta'

# 4. Decoradores para proteger rutas
@login_required
def ruta_protegida():
    # Solo accesible si hay sesión activa
    pass

@admin_required
def ruta_admin():
    # Solo para usuarios con rol admin
    pass
```

**Endpoints principales:**

| Ruta | Método | Descripción | Requiere Auth |
|------|--------|-------------|---------------|
| `/` | GET | Dashboard principal | ✅ |
| `/login` | GET | Página de login | ❌ |
| `/api/login` | POST | Autenticación | ❌ |
| `/api/logout` | POST | Cerrar sesión | ✅ |
| `/api/entrada` | POST | Registrar entrada | ✅ |
| `/api/salida` | POST | Registrar salida | ✅ |
| `/api/historial` | GET | Obtener historial | ✅ |
| `/api/usuarios` | GET | Listar usuarios | ✅ Admin |
| `/api/usuarios/crear` | POST | Crear usuario | ✅ Admin |

**Flujo de una operación:**

```
Usuario en web → Hace POST /api/entrada
                      ↓
                 Flask valida sesión
                      ↓
                 Llama a C++: parqueadero.registrar_entrada()
                      ↓
                 C++ procesa y retorna resultado
                      ↓
                 Flask guarda en DB: db.registrar_entrada()
                      ↓
                 Retorna JSON al frontend
```

#### Archivo: `database.py`

Gestiona toda la persistencia con SQLite.

**Tablas:**

```sql
-- usuarios: Información de usuarios del sistema
CREATE TABLE usuarios (
    id INTEGER PRIMARY KEY,
    username TEXT UNIQUE NOT NULL,
    password_hash TEXT NOT NULL,
    nombre_completo TEXT NOT NULL,
    rol TEXT CHECK(rol IN ('admin', 'operador')),
    activo INTEGER DEFAULT 1
);

-- historial: Registro de todas las transacciones
CREATE TABLE historial (
    id INTEGER PRIMARY KEY,
    placa TEXT NOT NULL,
    tipo_vehiculo TEXT CHECK(tipo_vehiculo IN ('carro', 'moto')),
    espacio INTEGER NOT NULL,
    hora_entrada TEXT NOT NULL,
    hora_salida TEXT,
    tarifa REAL,
    usuario_entrada TEXT NOT NULL,
    usuario_salida TEXT,
    estado TEXT CHECK(estado IN ('activo', 'finalizado'))
);

-- sesiones: Log de inicios de sesión
CREATE TABLE sesiones (
    id INTEGER PRIMARY KEY,
    username TEXT NOT NULL,
    fecha_inicio TEXT DEFAULT CURRENT_TIMESTAMP,
    ip_address TEXT
);
```

**Métodos clave:**

```python
# Autenticación
usuario, error = db.autenticar_usuario(username, password)

# Guardar operaciones
db.registrar_entrada(placa, tipo, espacio, usuario)
db.registrar_salida(placa, tarifa, usuario)

# Consultas
historial = db.obtener_historial(fecha_inicio, fecha_fin, usuario)
stats = db.obtener_estadisticas(fecha_inicio, fecha_fin)
```

### 4. Simulador Automatizado

#### Archivo: `simulador.py`

Herramienta para generar tráfico automático y probar el sistema.

**Arquitectura del simulador:**

```python
class SimuladorParqueadero:
    def __init__(self, parqueadero):
        self.parqueadero = parqueadero
        self.placas_disponibles = [...]
        self.estadisticas = {...}
```

**Lógica adaptativa:**

```python
def decidir_accion(self):
    vehiculos_dentro = len(self.parqueadero.listar_vehiculos())
    
    if vehiculos_dentro == 0:
        return 'entrada'  # Solo meter si está vacío
    
    if espacios_totales == 0:
        return 'salida'  # Solo sacar si está lleno
    
    if vehiculos_dentro < 5:
        # 70% entrada, 30% salida (favorecer llenar)
        return random.choices(['entrada', 'salida'], 
                             weights=[70, 30])[0]
    
    if espacios_disponibles < 5:
        # 30% entrada, 70% salida (favorecer vaciar)
        return random.choices(['entrada', 'salida'], 
                             weights=[30, 70])[0]
    
    # 50-50 en estado medio
    return random.choice(['entrada', 'salida'])
```

**Características:**

1. **Evita duplicados:** Solo usa placas que no están dentro
2. **Respeta capacidad:** No intenta meter si está lleno
3. **Maneja errores:** Captura todas las excepciones
4. **Estadísticas:** Cuenta entradas/salidas exitosas/rechazadas
5. **Configurable:** Ajusta velocidad y cantidad de operaciones

**Uso:**

```python
# Simulación rápida (100 ops, sin espera)
simulador.ejecutar(num_operaciones=100, intervalo=0)

# Simulación lenta (30 ops, 2 seg entre cada una)
simulador.ejecutar(num_operaciones=30, intervalo=2)

# Simulación balanceada
simulador.ejecutar(num_operaciones=50, intervalo=0.5)
```

**Salida ejemplo:**

```
🚀 Iniciando simulación del parqueadero...

--- Operación #1 ---
✅ ENTRADA: ABC123 (CARRO) - Vehículo registrado en espacio 1

--- Operación #2 ---
✅ ENTRADA: DEF456 (MOTO) - Vehículo registrado en espacio 1

--- Operación #3 ---
🚗 SALIDA: ABC123 - Tarifa: $3,000

📊 ESTADÍSTICAS FINALES
✅ Entradas exitosas: 45
❌ Entradas rechazadas: 5
🚗 Salidas exitosas: 40
💰 Total recaudado: $120,000
```

## 🚀 Uso

### Iniciar la Aplicación Web

```bash
python app.py
```

Accede a: **http://localhost:5000**

**Credenciales por defecto:**
- Usuario: `admin`
- Contraseña: `admin123`

### Ejecutar el Simulador

```bash
python simulador.py
```

### Usar el módulo C++ directamente

```python
import parqueadero_cpp

# Crear parqueadero
p = parqueadero_cpp.Parqueadero(10, 20, 3000, 2000)

# Operaciones
print(p.registrar_entrada("ABC123", "carro"))
print(p.espacios_disponibles_carros())
print(p.calcular_tarifa("ABC123"))
print(p.registrar_salida("ABC123"))
```

## 📚 API Reference

### Clase Parqueadero (C++)

#### Constructor
```python
Parqueadero(cap_carros: int, cap_motos: int, 
            tarifa_carro: float = 3000.0, 
            tarifa_moto: float = 2000.0)
```

#### Métodos

| Método | Parámetros | Retorno | Descripción |
|--------|-----------|---------|-------------|
| `registrar_entrada` | `placa: str, tipo: str` | `str` | Registra entrada de vehículo |
| `registrar_salida` | `placa: str` | `str` | Registra salida y retorna mensaje con tarifa |
| `vehiculo_presente` | `placa: str` | `bool` | Verifica si vehículo está dentro |
| `espacios_disponibles_carros` | - | `int` | Espacios libres para carros |
| `espacios_disponibles_motos` | - | `int` | Espacios libres para motos |
| `listar_vehiculos` | - | `list[str]` | Lista de placas dentro |
| `info_vehiculo` | `placa: str` | `str` | Información detallada formateada |
| `calcular_tarifa` | `placa: str` | `float` | Tarifa actual del vehículo |

## ⚙️ Configuración

### Modificar capacidad y tarifas

En `app.py`:
```python
parqueadero = parqueadero_cpp.Parqueadero(
    cap_carros=20,      # Capacidad de carros
    cap_motos=30,       # Capacidad de motos
    tarifa_carro=3000,  # $/hora carros
    tarifa_moto=2000    # $/hora motos
)
```

### Cambiar clave secreta de Flask

En `app.py`, línea 9:
```python
app.secret_key = 'tu-clave-super-segura-aqui'
```

### Ajustar puerto del servidor

En `app.py`, última línea:
```python
app.run(debug=True, port=5000)  # Cambiar 5000 por otro puerto
```

## 🐛 Troubleshooting

### Error: "No module named 'parqueadero_cpp'"
**Solución:** Compilar el módulo C++
```bash
make
```

### Error al compilar en Windows
**Solución:** Instalar compilador MinGW o usar Visual Studio
```bash
# Con chocolatey
choco install mingw

# O descargar de: https://sourceforge.net/projects/mingw/
```

### Error: "Unable to find vcvarsall.bat" (Windows)
**Solución:** Instalar Visual Studio Build Tools
- Descargar: https://visualstudio.microsoft.com/downloads/
- Instalar: "C++ build tools"

### Puerto 5000 ya en uso
**Solución:** Cambiar puerto en app.py o matar proceso
```bash
# Linux/Mac
lsof -ti:5000 | xargs kill -9

# Windows
netstat -ano | findstr :5000
taskkill /PID <PID> /F
```

## 🤝 Contribuir

¡Las contribuciones son bienvenidas! Por favor:

1. Fork el proyecto
2. Crea una rama para tu feature (`git checkout -b feature/AmazingFeature`)
3. Commit tus cambios (`git commit -m 'Add some AmazingFeature'`)
4. Push a la rama (`git push origin feature/AmazingFeature`)
5. Abre un Pull Request



## 🙏 Agradecimientos

- pybind11 por hacer fácil la integración C++/Python
- Flask por el excelente framework web
- La comunidad de código abierto

---

⭐ Si este proyecto te fue útil, considera darle una estrella en GitHub!
