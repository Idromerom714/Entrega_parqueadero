#!/usr/bin/env python3
"""
simulador/simulador.py
Simulador del parqueadero. Si no existe el binding C++ (parqueadero_cpp),
usa una clase "mock" en Python para permitir ejecutar y probar el simulador.
"""
import random
import time

# Intentar importar el módulo compilado; si falla, usar un mock para permitir pruebas.
try:
    import parqueadero_cpp  # Este viene de los bindings en C++
    BINDINGS_OK = True
except Exception:
    BINDINGS_OK = False

# Placas disponibles para las simulaciones
PLACAS_DISPONIBLES = [
    'ABC123', 'DEF456', 'GHI789', 'JKL012', 'MNO345',
    'PQR678', 'STU901', 'VWX234', 'YZA567', 'BCD890',
    'EFG123', 'HIJ456', 'KLM789', 'NOP012', 'QRS345',
    'TUV678', 'WXY901', 'ZAB234', 'CDE567', 'FGH890'
]

# ----------------------------
# Mock ligero del parqueadero (solo si no existe el binding)
# ----------------------------
if not BINDINGS_OK:
    import time as _time
    class _MockParqueadero:
        def __init__(self, cap_carros, cap_motos, tarifa_carro=3000.0, tarifa_moto=2000.0):
            self.cap_carros = cap_carros
            self.cap_motos = cap_motos
            self.tarifa_carro = tarifa_carro
            self.tarifa_moto = tarifa_moto
            self.vehiculos = {}  # placa -> (tipo,hora,espacio)
            self.espacios_carros = [False]*cap_carros
            self.espacios_motos = [False]*cap_motos

        def espacios_disponibles_carros(self):
            return sum(1 for x in self.espacios_carros if not x)

        def espacios_disponibles_motos(self):
            return sum(1 for x in self.espacios_motos if not x)

        def listar_vehiculos(self):
            return list(self.vehiculos.keys())

        def vehiculo_presente(self, placa):
            return placa in self.vehiculos

        def _asignar_espacio(self, tipo):
            arr = self.espacios_carros if tipo == 'carro' else self.espacios_motos
            for i, ocupado in enumerate(arr):
                if not ocupado:
                    arr[i] = True
                    return i+1
            return -1

        def _liberar_espacio(self, tipo, espacio):
            idx = espacio-1
            if tipo == 'carro':
                self.espacios_carros[idx] = False
            else:
                self.espacios_motos[idx] = False

        def registrar_entrada(self, placa, tipo):
            if self.vehiculo_presente(placa):
                return f"ERROR: El vehículo {placa} ya está dentro"
            espacio = self._asignar_espacio(tipo)
            if espacio == -1:
                return f"ERROR: No hay espacios para {tipo}"
            self.vehiculos[placa] = (tipo, int(_time.time()), espacio)
            return f"OK: Vehículo {placa} registrado en espacio {espacio}"

        def calcular_tarifa(self, placa):
            if placa not in self.vehiculos:
                return 0.0
            tipo, hora_entrada, _ = self.vehiculos[placa]
            ahora = int(_time.time())
            segundos = ahora - hora_entrada
            horas = (segundos / 3600.0)
            horas_cobradas = int(-(-horas // 1)) if horas > 0 else 1  # ceil manual
            tarifa_hora = self.tarifa_carro if tipo == 'carro' else self.tarifa_moto
            return horas_cobradas * tarifa_hora

        def registrar_salida(self, placa):
            if placa not in self.vehiculos:
                return f"ERROR: Vehículo {placa} no encontrado"
            tipo, hora_entrada, espacio = self.vehiculos[placa]
            tarifa = self.calcular_tarifa(placa)
            self._liberar_espacio(tipo, espacio)
            del self.vehiculos[placa]
            return f"OK: Vehículo {placa} retirado. Tarifa: ${tarifa:,.0f}"

    parqueadero_cpp = None  # variable no usada cuando mock se crea abajo
    Parqueadero = _MockParqueadero
else:
    # Usar la clase que exporta el binding C++ si está disponible
    Parqueadero = parqueadero_cpp.Parqueadero  # type: ignore

# ----------------------------
# SimuladorPrincipal
# ----------------------------
class SimuladorParqueadero:
    def __init__(self, parqueadero):
        self.parqueadero = parqueadero
        self.placas_disponibles = PLACAS_DISPONIBLES.copy()
        self.estadisticas = {
            'entradas_exitosas': 0,
            'entradas_rechazadas': 0,
            'salidas_exitosas': 0,
            'salidas_rechazadas': 0,
            'total_recaudado': 0.0
        }

    def obtener_placa_disponible(self):
        vehiculos_dentro = self.parqueadero.listar_vehiculos()
        libres = [p for p in self.placas_disponibles if p not in vehiculos_dentro]
        return random.choice(libres) if libres else None

    def obtener_tipo_vehiculo(self):
        return random.choice(['carro', 'moto'])

    def simular_entrada(self):
        espacios_carros = self.parqueadero.espacios_disponibles_carros()
        espacios_motos = self.parqueadero.espacios_disponibles_motos()
        if espacios_carros == 0 and espacios_motos == 0:
            print("❌ Parqueadero LLENO - No se puede registrar entrada")
            self.estadisticas['entradas_rechazadas'] += 1
            return
        placa = self.obtener_placa_disponible()
        if not placa:
            print("❌ No hay placas disponibles")
            self.estadisticas['entradas_rechazadas'] += 1
            return
        if espacios_carros == 0:
            tipo = 'moto'
        elif espacios_motos == 0:
            tipo = 'carro'
        else:
            tipo = self.obtener_tipo_vehiculo()
        resultado = self.parqueadero.registrar_entrada(placa, tipo)
        if resultado.startswith('OK'):
            print(f"✅ ENTRADA: {placa} ({tipo.upper()}) - {resultado[4:]}")
            self.estadisticas['entradas_exitosas'] += 1
        else:
            print(f"❌ ENTRADA RECHAZADA: {resultado}")
            self.estadisticas['entradas_rechazadas'] += 1

    def simular_salida(self):
        vehiculos = self.parqueadero.listar_vehiculos()
        if not vehiculos:
            print("⚠️ No hay vehículos para retirar")
            self.estadisticas['salidas_rechazadas'] += 1
            return
        placa = random.choice(vehiculos)
        tarifa = self.parqueadero.calcular_tarifa(placa)
        resultado = self.parqueadero.registrar_salida(placa)
        if resultado.startswith('OK'):
            print(f"🚗 SALIDA: {placa} - Tarifa: ${tarifa:,.0f}")
            self.estadisticas['salidas_exitosas'] += 1
            self.estadisticas['total_recaudado'] += tarifa
        else:
            print(f"❌ SALIDA RECHAZADA: {resultado}")
            self.estadisticas['salidas_rechazadas'] += 1

    def decidir_accion(self):
        vehiculos = len(self.parqueadero.listar_vehiculos())
        espacios = (self.parqueadero.espacios_disponibles_carros() +
                    self.parqueadero.espacios_disponibles_motos())
        if vehiculos == 0:
            return 'entrada'
        if espacios == 0:
            return 'salida'
        if vehiculos < 5:
            return random.choices(['entrada', 'salida'], weights=[70, 30])[0]
        if espacios < 5:
            return random.choices(['entrada', 'salida'], weights=[30, 70])[0]
        return random.choice(['entrada', 'salida'])

    def mostrar_estado(self):
        print("\n" + "=" * 60)
        print("📊 ESTADO DEL PARQUEADERO")
        print("=" * 60)
        print(f"🚗 Espacios CARROS: {self.parqueadero.espacios_disponibles_carros()}")
        print(f"🏍️  Espacios MOTOS: {self.parqueadero.espacios_disponibles_motos()}")
        print(f"📍 Vehículos dentro: {len(self.parqueadero.listar_vehiculos())}")
        print("=" * 60)

    def mostrar_estadisticas_finales(self):
        print("\n" + "=" * 60)
        print("📈 ESTADÍSTICAS FINALES")
        print("=" * 60)
        for k, v in self.estadisticas.items():
            if k == 'total_recaudado':
                print(f"{k.replace('_',' ').title()}: ${v:,.0f}")
            else:
                print(f"{k.replace('_',' ').title()}: {v}")
        print("=" * 60)

    def ejecutar(self, num_operaciones=50, intervalo=1.0):
        print("🚀 Iniciando simulación…")
        self.mostrar_estado()
        try:
            for i in range(1, num_operaciones + 1):
                print(f"\n--- Operación #{i} ---")
                accion = self.decidir_accion()
                if accion == 'entrada':
                    self.simular_entrada()
                else:
                    self.simular_salida()
                if i % 10 == 0:
                    self.mostrar_estado()
                if intervalo > 0:
                    time.sleep(intervalo)
        except KeyboardInterrupt:
            print("\n⚠️ Simulación interrumpida por el usuario")
        finally:
            self.mostrar_estado()
            self.mostrar_estadisticas_finales()
            restantes = self.parqueadero.listar_vehiculos()
            if restantes:
                print("\nVehículos que quedaron dentro:")
                for placa in restantes:
                    tarifa = self.parqueadero.calcular_tarifa(placa)
                    print(f"• {placa} → Tarifa actual: ${tarifa:,.0f}")

def main():
    print("🏗️ Creando parqueadero…")
    # Si se compiló el binding C++, Parqueadero vendrá de ahí.
    # Si no, Parqueadero es la clase mock definida arriba.
    p = Parqueadero(20, 30, 3000.0, 2000.0)
    simulador = SimuladorParqueadero(p)
    simulador.ejecutar(num_operaciones=50, intervalo=0.5)

if __name__ == "__main__":
    main()
