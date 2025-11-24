#!/usr/bin/env python3
"""
Servidor IoT para Sistema de Parqueadero
Recibe eventos de dispositivos remotos (cámaras de placas)
"""

import parqueadero_cpp
import threading
import time
from datetime import datetime
from database import Database

class ServidorIoT:
    def __init__(self, capacidad_carros=20, capacidad_motos=30, puerto=8080):
        # Crear parqueadero
        self.parqueadero = parqueadero_cpp.Parqueadero(
            capacidad_carros, 
            capacidad_motos, 
            3000.0, 
            2000.0
        )
        
        # Crear servidor TCP/IP
        self.servidor = parqueadero_cpp.ServidorParqueadero(self.parqueadero, puerto)
        
        # Base de datos para persistencia
        self.db = Database()
        
        # Estado
        self.ejecutando = False
        self.puerto = puerto
        self.eventos_procesados = 0
        self.thread_servidor = None
        
        # Configurar callback para eventos
        self.servidor.establecer_callback(self._manejar_evento)
    
    def _manejar_evento(self, tipo, placa, tipo_vehiculo, exito):
        """
        Callback llamado cuando un dispositivo envía un evento
        tipo: "ENTRADA" o "SALIDA"
        placa: placa del vehículo
        tipo_vehiculo: "carro" o "moto"
        exito: True si la operación fue exitosa
        """
        self.eventos_procesados += 1
        timestamp = datetime.now().strftime('%Y-%m-%d %H:%M:%S')
        
        print(f"\n{'='*60}")
        print(f"🔔 EVENTO #{self.eventos_procesados}")
        print(f"{'='*60}")
        print(f"Tipo:     {tipo}")
        print(f"Placa:    {placa}")
        print(f"Vehículo: {tipo_vehiculo}")
        print(f"Estado:   {'✅ ÉXITO' if exito else '❌ RECHAZADO'}")
        print(f"Hora:     {timestamp}")
        print(f"{'='*60}\n")
        
        # Guardar en base de datos si fue exitoso
        if exito and tipo == "ENTRADA":
            # Obtener info del vehículo para saber el espacio
            info = self.parqueadero.info_vehiculo(placa)
            # Parsear espacio de la info (formato: "Espacio: N")
            espacio = 0
            for linea in info.split('\n'):
                if linea.startswith('Espacio:'):
                    espacio = int(linea.split(':')[1].strip())
                    break
            
            self.db.registrar_entrada(placa, tipo_vehiculo, espacio, "dispositivo_iot")
        
        elif exito and tipo == "SALIDA":
            tarifa = self.parqueadero.calcular_tarifa(placa)
            self.db.registrar_salida(placa, tarifa, "dispositivo_iot")
    
    def _loop_servidor(self):
        """Loop principal del servidor que acepta conexiones"""
        while self.ejecutando:
            try:
                # Aceptar conexión (bloquea hasta recibir una)
                if not self.servidor.aceptar_conexion():
                    if self.ejecutando:  # Solo mostrar error si aún está ejecutando
                        print("⚠️  Error al aceptar conexión")
            except Exception as e:
                if self.ejecutando:
                    print(f"❌ Error en servidor: {e}")
                break
    
    def iniciar(self):
        """Inicia el servidor IoT"""
        if self.ejecutando:
            print("⚠️  El servidor ya está ejecutando")
            return False
        
        print("\n" + "="*60)
        print("🚀 INICIANDO SERVIDOR IoT")
        print("="*60)
        
        if not self.servidor.iniciar():
            print("❌ Error al iniciar servidor")
            return False
        
        self.ejecutando = True
        
        # Iniciar thread para aceptar conexiones
        self.thread_servidor = threading.Thread(target=self._loop_servidor, daemon=True)
        self.thread_servidor.start()
        
        print(f"✅ Servidor escuchando en puerto {self.puerto}")
        print(f"📡 Esperando dispositivos IoT...")
        print(f"🅿️  Capacidad: {self.parqueadero.espacios_disponibles_carros()} carros, "
              f"{self.parqueadero.espacios_disponibles_motos()} motos")
        print("="*60 + "\n")
        
        return True
    
    def detener(self):
        """Detiene el servidor"""
        if not self.ejecutando:
            return
        
        print("\n" + "="*60)
        print("🛑 DETENIENDO SERVIDOR")
        print("="*60)
        
        self.ejecutando = False
        self.servidor.detener()
        
        if self.thread_servidor:
            self.thread_servidor.join(timeout=2)
        
        print(f"📊 Total eventos procesados: {self.eventos_procesados}")
        print("="*60 + "\n")
    
    def mostrar_estado(self):
        """Muestra el estado actual del parqueadero"""
        print("\n" + "="*60)
        print("📊 ESTADO DEL PARQUEADERO")
        print("="*60)
        print(f"🚗 Espacios carros disponibles: {self.parqueadero.espacios_disponibles_carros()}")
        print(f"🏍️  Espacios motos disponibles:  {self.parqueadero.espacios_disponibles_motos()}")
        print(f"📍 Vehículos dentro: {len(self.parqueadero.listar_vehiculos())}")
        print(f"🔔 Eventos procesados: {self.eventos_procesados}")
        
        vehiculos = self.parqueadero.listar_vehiculos()
        if vehiculos:
            print("\n📋 Vehículos actuales:")
            for placa in vehiculos:
                tarifa = self.parqueadero.calcular_tarifa(placa)
                print(f"   • {placa} - Tarifa actual: ${tarifa:,.0f}")
        
        print("="*60 + "\n")
    
    def ejecutar_interactivo(self):
        """Modo interactivo con menú"""
        try:
            while True:
                print("\n┌─────────────────────────────────┐")
                print("│  SERVIDOR IoT - MENÚ            │")
                print("├─────────────────────────────────┤")
                print("│  1. Mostrar estado              │")
                print("│  2. Listar vehículos            │")
                print("│  3. Info de vehículo            │")
                print("│  4. Estadísticas                │")
                print("│  5. Salir                       │")
                print("└─────────────────────────────────┘")
                
                opcion = input("Opción: ").strip()
                
                if opcion == "1":
                    self.mostrar_estado()
                
                elif opcion == "2":
                    vehiculos = self.parqueadero.listar_vehiculos()
                    if vehiculos:
                        print("\n📋 Vehículos en el parqueadero:")
                        for placa in vehiculos:
                            print(f"   • {placa}")
                    else:
                        print("\n⚠️  No hay vehículos")
                
                elif opcion == "3":
                    placa = input("Placa: ").strip().upper()
                    if self.parqueadero.vehiculo_presente(placa):
                        info = self.parqueadero.info_vehiculo(placa)
                        print(f"\n{info}")
                    else:
                        print(f"\n❌ Vehículo {placa} no encontrado")
                
                elif opcion == "4":
                    stats = self.db.obtener_estadisticas()
                    print("\n" + "="*40)
                    print("📈 ESTADÍSTICAS")
                    print("="*40)
                    print(f"Total vehículos: {stats['total_vehiculos']}")
                    print(f"Total carros: {stats['total_carros']}")
                    print(f"Total motos: {stats['total_motos']}")
                    print(f"Recaudado: ${stats['total_recaudado']:,.0f}")
                    print("="*40)
                
                elif opcion == "5":
                    break
                
                else:
                    print("❌ Opción inválida")
        
        except KeyboardInterrupt:
            print("\n\n⚠️  Interrupción detectada")
        
        finally:
            self.detener()


def main():
    print("╔════════════════════════════════════════╗")
    print("║  Sistema de Parqueadero IoT            ║")
    print("║  Servidor de Dispositivos Remotos      ║")
    print("╚════════════════════════════════════════╝\n")
    
    # Crear y configurar servidor
    servidor = ServidorIoT(capacidad_carros=20, capacidad_motos=30, puerto=8080)
    
    # Iniciar servidor
    if not servidor.iniciar():
        print("❌ No se pudo iniciar el servidor")
        return
    
    try:
        print("💡 Tip: Ejecuta el cliente en otra terminal:")
        print("   Linux/Mac: ./cliente_dispositivo")
        print("   Windows:   cliente_dispositivo.exe")
        print("\n⌨️  Presiona Ctrl+C para menú interactivo\n")
        
        # Esperar un poco antes de entrar al menú
        time.sleep(2)
        
        # Modo interactivo
        servidor.ejecutar_interactivo()
    
    except KeyboardInterrupt:
        print("\n\n⚠️  Deteniendo servidor...")
        servidor.detener()
    
    print("\n👋 Servidor finalizado")


if __name__ == "__main__":
    main()
