# Simulador Automatizado del Parqueadero

El simulador genera operaciones automáticas (entradas/salidas) para testing, demos y generación de datos.

## Contenido
- Arquitectura del simulador
- Lógica adaptativa
- Estadísticas y métricas
- Modos de ejecución
- Personalización
- Tips y trucos

## Arquitectura
SimuladorParqueadero
- __init__(parqueadero)
- obtener_placa_disponible()
- obtener_tipo_vehiculo()
- simular_entrada()
- simular_salida()
- decidir_accion()
- mostrar_estado()
- mostrar_estadisticas_finales()
- ejecutar(num_ops, intervalo)

## Lógica adaptativa
- Si está vacío → solo entradas
- Si está lleno → solo salidas
- Vehículos <5 → favorece entradas (70%)
- Espacios <5 → favorece salidas (70%)
- Medio → 50/50

## Estadísticas
- entradas_exitosas
- entradas_rechazadas
- salidas_exitosas
- salidas_rechazadas
- total_recaudado

## Modos de ejecución
- Rápido: ejecutar(200, 0)
- Demo: ejecutar(30, 2)
- Balanceado: ejecutar(50, 0.5)

## Personalización
- Cambiar placas, pesos, delays, etc.
- Reproducibilidad con random.seed()

