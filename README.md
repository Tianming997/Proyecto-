# Monitor de Calidad del Aire IoT (ESP32 + BME680)

Este proyecto consiste en un sistema embebido de monitoreo ambiental e IoT desarrollado para la asignatura de **Procesadores Digitales (UPC)**. El sistema mide en tiempo real la temperatura, humedad y compuestos orgánicos volátiles (gases COV), muestra los datos en una pantalla local y los transmite a través de MQTT mediante Wi-Fi. Además, cuenta con un sistema de alerta acústica.

## 🚀 Funcionalidades

- **Monitoreo Ambiental:** Lectura continua de temperatura, humedad y gases con el sensor Bosch BME680.
- **Interfaz Local:** Visualización de los datos en tiempo real en una pantalla OLED SH1106 (1.3").
- **Conectividad IoT:** Conexión Wi-Fi y envío de telemetría mediante el protocolo MQTT.
- **Alerta Acústica:** Activación automática de un zumbador activo cuando la calidad del aire baja de un umbral seguro.
- **Eficiencia Energética:** Implementación de modo Deep-Sleep para optimizar el consumo de batería.

## 🛠️ Hardware Utilizado

- **Microcontrolador:** ESP32 WROOM-32D (DevKitC)
- **Sensor:** Bosch BME680 (I2C)
- **Pantalla:** OLED SH1106 128x64 (I2C)
- **Actuador de Alerta:** Zumbador Activo 5V/3.3V
- **Entorno de desarrollo:** PlatformIO en VS Code

## 🔌 Conexiones (Pinout)

Tanto la pantalla como el sensor comparten la misma línea de datos física (Autopista I2C) en los pines estándar del ESP32.

| Componente | Pin Componente | Pin ESP32 | Descripción |
| :--- | :--- | :--- | :--- |
| **OLED SH1106** | VCC / GND | VIN (5V) / GND | Alimentación de la pantalla |
| **OLED SH1106** | SDA / SCL | GPIO21 / GPIO22 | Bus I2C (Compartido) |
| **BME680** | VCC / GND | 3.3V / GND | Alimentación del sensor (¡Delicado!) |
| **BME680** | SDA / SCL | GPIO21 / GPIO22 | Bus I2C (Compartido) |
| **BME680** | CS / SDO | 3.3V / 3.3V | Configura modo I2C y dirección `0x77` |
| **Zumbador** | Positivo (+) | GPIO4 | Señal digital de control de alarma |
| **Zumbador** | Negativo (-) | GND | Tierra |

## 📊 Diagrama de Bloques

*(Aquí puedes pegar la imagen PNG que exportaste de Mermaid)*
![Diagrama de Bloques](./diagrama_ejemplo_estacion_meteo.png)

<details>
<summary><b>Haz clic aquí para ver el código fuente de Mermaid</b></summary>

```mermaid
fflowchart TB
    %% (Pega aquí el código de Mermaid que te pasé en la respuesta anterior)
