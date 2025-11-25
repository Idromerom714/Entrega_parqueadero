# Makefile multiplataforma para el sistema de parqueadero

# Detectar sistema operativo
ifeq ($(OS),Windows_NT)
    PLATFORM := Windows
    CXX := g++
    PYTHON := python
    RM := del /Q
    MKDIR := if not exist
    SUFFIX := .pyd
    SOCKET_LIBS := -lws2_32
else
    UNAME_S := $(shell uname -s)
    ifeq ($(UNAME_S),Linux)
        PLATFORM := Linux
    endif
    ifeq ($(UNAME_S),Darwin)
        PLATFORM := macOS
    endif
    CXX := g++
    PYTHON := python3
    RM := rm -f
    MKDIR := mkdir -p
    SUFFIX := $(shell $(PYTHON)-config --extension-suffix)
    SOCKET_LIBS :=
endif

# Flags de compilación
CXXFLAGS := -O3 -Wall -shared -std=c++11 -fPIC
INCLUDES := $(shell $(PYTHON) -m pybind11 --includes) -Icpp

# Archivos
MODULE := parqueadero_cpp$(SUFFIX)
CLIENTE := cliente_dispositivo
SOURCES := cpp/parqueadero.cpp cpp/socket_utils.cpp cpp/servidor_parqueadero.cpp cpp/bindings.cpp
CLIENTE_SRC := cpp/cliente_dispositivo.cpp cpp/socket_utils.cpp

# Agregar extensión .exe en Windows
ifeq ($(PLATFORM),Windows)
    CLIENTE := $(CLIENTE).exe
endif

.PHONY: all module cliente clean test help

# Target por defecto
all: module cliente

# Compilar módulo Python
module: $(MODULE)

$(MODULE): $(SOURCES)
	@echo "🔨 Compilando módulo Python para $(PLATFORM)..."
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(SOURCES) -o $(MODULE) $(SOCKET_LIBS)
	@echo "✅ Módulo compilado: $(MODULE)"

# Compilar cliente (dispositivo simulador)
cliente: $(CLIENTE)

$(CLIENTE): $(CLIENTE_SRC)
	@echo "🔨 Compilando cliente dispositivo para $(PLATFORM)..."
	$(CXX) -O3 -Wall -std=c++11 $(CLIENTE_SRC) -o $(CLIENTE) -I. -Icpp $(SOCKET_LIBS)
	@echo "✅ Cliente compilado: $(CLIENTE)"

# Limpiar archivos compilados
clean:
ifeq ($(PLATFORM),Windows)
	@echo "🧹 Limpiando archivos..."
	-$(RM) $(MODULE) 2>nul
	-$(RM) $(CLIENTE) 2>nul
	-$(RM) *.o 2>nul
else
	@echo "🧹 Limpiando archivos..."
	$(RM) $(MODULE) $(CLIENTE) *.o
endif
	@echo "✅ Limpieza completada"

# Probar módulo Python
test: $(MODULE)
	@echo "🧪 Probando módulo..."
	$(PYTHON) -c "import parqueadero_cpp; p = parqueadero_cpp.Parqueadero(10, 20); print('✅ Módulo funcionando correctamente')"

# Ejecutar aplicación
run: $(MODULE)
	@echo "🚀 Ejecutando aplicación..."
	$(PYTHON) app.py

# Ejecutar cliente en modo interactivo
run-cliente: $(CLIENTE)
	@echo "🤖 Ejecutando cliente dispositivo..."
ifeq ($(PLATFORM),Windows)
	$(CLIENTE)
else
	./$(CLIENTE)
endif

# Ejecutar cliente en modo automático
run-cliente-auto: $(CLIENTE)
	@echo "🤖 Ejecutando cliente en modo automático..."
ifeq ($(PLATFORM),Windows)
	$(CLIENTE) CAMARA-01 127.0.0.1 8080 auto 20
else
	./$(CLIENTE) CAMARA-01 127.0.0.1 8080 auto 20
endif

# Ayuda
help:
	@echo "════════════════════════════════════════════════"
	@echo "  Makefile - Sistema de Parqueadero ($(PLATFORM))"
	@echo "════════════════════════════════════════════════"
	@echo ""
	@echo "Targets disponibles:"
	@echo "  make              - Compila todo (módulo + cliente)"
	@echo "  make module       - Compila solo módulo Python"
	@echo "  make cliente      - Compila solo cliente dispositivo"
	@echo "  make clean        - Elimina archivos compilados"
	@echo "  make test         - Prueba el módulo Python"
	@echo "  make run          - Ejecuta la aplicación Flask"
	@echo "  make run-cliente  - Ejecuta cliente modo interactivo"
	@echo "  make run-cliente-auto - Ejecuta cliente modo automático"
	@echo "  make help         - Muestra esta ayuda"
	@echo ""