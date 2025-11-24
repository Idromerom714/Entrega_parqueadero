from flask import Flask, render_template, request, jsonify
import parqueadero_cpp
import placas
import time

app = Flask(__name__)

# Crear instancia del parqueadero (20 carros, 30 motos)
parqueadero = parqueadero_cpp.Parqueadero(20, 30, 3000.0, 2000.0)

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

if __name__ == '__main__':
    app.run(debug=True, port=5000)