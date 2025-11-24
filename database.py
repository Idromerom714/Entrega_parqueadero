from datetime import datetime

class Database:
    """Implementación simple en memoria para pruebas.
    Guarda un historial básico de entradas/salidas y calcula estadísticas.
    """
    def __init__(self):
        self.historial = []  # lista de dicts: {placa,tipo,entrada,salida,tarifa,fuente}

    def registrar_entrada(self, placa, tipo, espacio, fuente):
        now = datetime.now().isoformat(sep=' ')
        self.historial.append({
            'placa': placa,
            'tipo': tipo,
            'hora_entrada': now,
            'hora_salida': None,
            'tarifa': 0.0,
            'espacio': espacio,
            'fuente': fuente,
        })

    def registrar_salida(self, placa, tarifa, fuente):
        now = datetime.now().isoformat(sep=' ')
        # buscar última entrada sin salida para esta placa
        for rec in reversed(self.historial):
            if rec['placa'] == placa and rec['hora_salida'] is None:
                rec['hora_salida'] = now
                rec['tarifa'] = float(tarifa)
                rec['fuente_salida'] = fuente
                return True
        # si no existe, agregar un registro de salida solitario
        self.historial.append({
            'placa': placa,
            'tipo': None,
            'hora_entrada': None,
            'hora_salida': now,
            'tarifa': float(tarifa),
            'espacio': None,
            'fuente': fuente,
        })
        return False

    def obtener_estadisticas(self):
        total_vehiculos = 0
        total_carros = 0
        total_motos = 0
        total_recaudado = 0.0

        placas_actuales = set()

        for rec in self.historial:
            if rec.get('hora_entrada') and not rec.get('hora_salida'):
                placas_actuales.add(rec['placa'])

            if rec.get('tarifa'):
                total_recaudado += float(rec.get('tarifa', 0.0))

            if rec.get('tipo') == 'carro':
                total_carros += 1
            elif rec.get('tipo') == 'moto':
                total_motos += 1

        total_vehiculos = len(placas_actuales)

        return {
            'total_vehiculos': total_vehiculos,
            'total_carros': total_carros,
            'total_motos': total_motos,
            'total_recaudado': total_recaudado,
        }
