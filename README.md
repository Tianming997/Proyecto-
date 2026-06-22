# Monitor de Calidad del Aire IoT (ESP32 + BME680)

Este proyecto consiste en un sistema embebido de monitoreo ambiental e IoT desarrollado para la asignatura de **Procesadores Digitales (UPC)**. El sistema mide en tiempo real la temperatura, humedad y compuestos orgánicos volátiles (gases COV), muestra los datos en una pantalla local y los transmite a la nube mediante Wi-Fi y el protocolo MQTT.

## 🚀 Funcionalidades

- **Monitoreo Ambiental:** Lectura continua de temperatura, humedad y gases con el sensor Bosch BME680.
- **Interfaz Local:** Visualización de los datos en tiempo real en una pantalla OLED SH1106 (1.3").
- **Conectividad IoT:** Conexión Wi-Fi y envío de telemetría asíncrona cada 10 segundos al broker público HiveMQ mediante MQTT.
- **Alerta Visual Inteligente:** Si la calidad del aire baja de un umbral seguro (< 50 kΩ), la interfaz cambia a modo emergencia (`"¡VENTILAR SALA!"`) y transmite la alerta a la nube.
- **Control Térmico:** Implementación de una barrera de *warm-up* de 3 minutos por software para estabilizar el calentador del sensor antes de permitir el disparo de alertas.

## 🛠️ Hardware Utilizado

- **Microcontrolador:** ESP32 WROOM-32D (DevKitC)
- **Sensor:** Módulo Bosch BME680 (I2C)
- **Actuador Visual:** Pantalla OLED SH1106 128x64 (I2C)
- **Entorno de Desarrollo:** PlatformIO en VS Code

## 🔌 Conexiones (Pinout)

Tanto la pantalla como el sensor comparten la misma línea de datos física (bus I2C) en los pines estándar del ESP32, optimizando el uso del hardware.

| Componente | Pin Componente | Pin ESP32 | Descripción |
| :--- | :--- | :--- | :--- |
| **OLED SH1106** | VCC / GND | VIN (5V) / GND | Alimentación de la pantalla |
| **OLED SH1106** | SDA / SCL | GPIO21 / GPIO22 | Bus I2C (Compartido) |
| **BME680** | VCC / GND | 3.3V / GND | Alimentación del sensor (Estricto a 3.3V) |
| **BME680** | SDA / SCL | GPIO21 / GPIO22 | Bus I2C (Compartido) |
| **BME680** | CS / SDO | 3.3V / 3.3V | Configura modo I2C y fija dirección `0x77` |

## 📊 Diagrama de Bloques

![Diagrama de Bloques](./diagrama_bloques.png)
*(Nota: Asegúrate de subir la nueva imagen exportada de Mermaid con el nombre `diagrama_bloques.png` a tu repositorio).*

<details>
<summary><b>Ver código de Mermaid</b></summary>

```mermaid
flowchart TB
    subgraph Alimentacion [Alimentación]
        USB[Entrada USB 5V] -->|5V| VIN[Línea 5V / VIN]
        USB -->|5V| REG[Regulador Interno AMS1117]
        REG -->|3.3V| V33[Línea 3.3V]
    end

    subgraph Central [Procesamiento]
        MCU{{ESP32 WROOM-32D}}
    end

    subgraph Sensores [Sensores]
        BME[Sensor BME680]
    end

    subgraph Interfaz [Interfaz de Usuario]
        OLED[OLED SH1106]
    end

    subgraph Red [Conectividad IoT]
        WIFI[Punto de Acceso Wi-Fi]
        MQT[Broker MQTT: HiveMQ]
    end

    VIN -->|5V| OLED
    V33 -->|3.3V| MCU
    V33 -->|3.3V| BME

    MCU -.->|"I²C: GPIO21 (SDA)"| OLED
    MCU -.->|"I²C: GPIO22 (SCL)"| OLED
    MCU -.->|"I²C: GPIO21 (SDA)"| BME
    MCU -.->|"I²C: GPIO22 (SCL)"| BME
    MCU -.->|"RF 2.4 GHz"| WIFI
    WIFI -.->|"Protocolo MQTT (10s)"| MQT

    classDef mcu fill:#d2e3fc,stroke:#1a73e8,stroke-width:2px;
    classDef sensor fill:#e6f4ea,stroke:#137333,stroke-width:2px;
    classDef power fill:#fef7e0,stroke:#b06000,stroke-width:2px;
    classDef io fill:#fce8e6,stroke:#c5221f,stroke-width:2px;
    classDef net fill:#f3e5f5,stroke:#7b1fa2,stroke-width:2px;

    class MCU mcu;
    class BME sensor;
    class USB,REG,VIN,V33 power;
    class OLED io;
    class WIFI,MQT net;
