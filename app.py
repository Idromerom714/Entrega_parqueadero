from flask import Flask, render_template, request, jsonify
import parqueadero_cpp
import time
import random
import threading

app = Flask(__name__)

# Crear instancia del parqueadero (20 carros, 30 motos)
parqueadero = parqueadero_cpp.Parqueadero(20, 30, 3000.0, 2000.0)

# 50 placas predefinidas
PLACAS_PREDEFINIDAS = [
    'ABC123', 'DEF456', 'GHI789', 'JKL012', 'MNO345',
    'PQR678', 'STU901', 'VWX234', 'YZA567', 'BCD890',
    'EFG123', 'HIJ456', 'KLM789', 'NOP012', 'QRS345',
    'TUV678', 'WXY901', 'ZAB234', 'CDE567', 'FGH890',
    'IJK123', 'LMN456', 'OPQ789', 'RST012', 'UVW345',
    'XYZ678', 'AAA111', 'BBB222', 'CCC333', 'DDD444',
    'EEE555', 'FFF666', 'GGG777', 'HHH888', 'III999',
    'JJJ000', 'KKK111', 'LLL222', 'MMM333', 'NNN444',
    'OOO555', 'PPP666', 'QQQ777', 'RRR888', 'SSS999',
    'TTT000', 'UUU111', 'VVV222', 'WWW333', 'XXX444'
]

# Variable para controlar el simulador
simulador_activo = True

@app.route('/')
def index():
    return render_template('index.html')

@app.route('/api/estado')
def estado():
    """Obtiene el estado actual del parqueadero"""
    return jsonify({
        'espacios_carros': parqueadero.espacios_disponibles_carros(),
        'espacios_motos': parqueadero.espacios_disponibles_motos(),
        'vehiculos': parqueadero.listar_vehiculos()
    })

@app.route('/api/entrada', methods=['POST'])
def registrar_entrada():
    """Registra la entrada de un vehículo"""
    data = request.json
    placa = data.get('placa', '').upper().strip()
    tipo = data.get('tipo', '').lower()
    
    if not placa or tipo not in ['carro', 'moto']:
        return jsonify({'error': 'Datos inválidos'}), 400
    
    resultado = parqueadero.registrar_entrada(placa, tipo)
    
    if resultado.startswith('ERROR'):
        return jsonify({'error': resultado[7:]}), 400
    
    return jsonify({'mensaje': resultado[4:]})





@app.route('/api/salida', methods=['POST'])
def registrar_salida():
    """Registra la salida de un vehículo"""
    data = request.json
    placa = data.get('placa', '').upper().strip()
    
    if not placa:
        return jsonify({'error': 'Placa requerida'}), 400
    
    resultado = parqueadero.registrar_salida(placa)
    
    if resultado.startswith('ERROR'):
        return jsonify({'error': resultado[7:]}), 400
    
    return jsonify({'mensaje': resultado[4:]})

@app.route('/api/vehiculo/<placa>')
def info_vehiculo(placa):
    """Obtiene información de un vehículo"""
    placa = placa.upper().strip()
    info = parqueadero.info_vehiculo(placa)
    
    if info.startswith('ERROR'):
        return jsonify({'error': info[7:]}), 404
    
    return jsonify({'info': info})

@app.route('/api/tarifa/<placa>')
def calcular_tarifa(placa):
    """Calcula la tarifa actual de un vehículo"""
    placa = placa.upper().strip()
    
    if not parqueadero.vehiculo_presente(placa):
        return jsonify({'error': 'Vehículo no encontrado'}), 404
    
    tarifa = parqueadero.calcular_tarifa(placa)
    return jsonify({'tarifa': tarifa})

def simulador_automatico():
    """Simulador que ejecuta operaciones automáticas cada 2 segundos"""
    global simulador_activo
    
    while simulador_activo:
        try:
            # Obtener vehículos dentro del parqueadero
            vehiculos_dentro = parqueadero.listar_vehiculos()
            espacios_carros = parqueadero.espacios_disponibles_carros()
            espacios_motos = parqueadero.espacios_disponibles_motos()
            
            # Decidir si hacer entrada o salida
            if len(vehiculos_dentro) == 0:
                # Si no hay vehículos, solo entrada
                accion = 'entrada'
            elif espacios_carros == 0 and espacios_motos == 0:
                # Si está lleno, solo salida
                accion = 'salida'
            else:
                # Decisión aleatoria: 50% entrada, 50% salida
                accion = random.choice(['entrada', 'salida'])
            
            if accion == 'entrada':
                # Seleccionar una placa aleatoria que NO esté dentro
                placas_disponibles = [p for p in PLACAS_PREDEFINIDAS if p not in vehiculos_dentro]
                if placas_disponibles:
                    placa = random.choice(placas_disponibles)
                    # Seleccionar tipo de vehículo aleatorio
                    if espacios_carros == 0:
                        tipo = 'moto'
                    elif espacios_motos == 0:
                        tipo = 'carro'
                    else:
                        tipo = random.choice(['carro', 'moto'])
                    
                    resultado = parqueadero.registrar_entrada(placa, tipo)
                    if resultado.startswith('OK'):
                        print(f"ENTRADA: {placa} ({tipo.upper()}) - {resultado[4:]}")
                    else:
                        print(f"ERROR ENTRADA: {resultado}")
            
            else:  # salida
                if vehiculos_dentro:
                    # Seleccionar un vehículo aleatorio que esté dentro
                    placa = random.choice(vehiculos_dentro)
                    
                    # Validación adicional: verificar que el vehículo realmente esté presente
                    if parqueadero.vehiculo_presente(placa):
                        resultado = parqueadero.registrar_salida(placa)
                        if resultado.startswith('OK'):
                            # Extraer tarifa del mensaje
                            print(f"SALIDA: {placa} - {resultado[4:]}")
                        else:
                            print(f"ERROR SALIDA: {resultado}")
                    else:
                        print(f"ADVERTENCIA: La placa {placa} no está presente (puede haber sido removida)")
                else:
                    # Esto no debería pasar porque la lógica arriba evita esto, pero por seguridad:
                    print("No hay vehículos para retirar")
            
        except Exception as e:
            print(f"Error: {e}")
        
        # Esperar 2 segundos antes de la siguiente operación
        time.sleep(2)

if __name__ == '__main__':
    # Iniciar simulador en un thread separado
    thread_simulador = threading.Thread(target=simulador_automatico, daemon=True)
    thread_simulador.start()
    print("Simulador automático iniciado (operaciones cada 2 segundos)")
    
    app.run(debug=True, port=5000)