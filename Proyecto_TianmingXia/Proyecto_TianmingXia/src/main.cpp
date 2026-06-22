#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <U8g2lib.h>
#include "Adafruit_BME680.h"

// ============================================================================
// 1. CONFIGURACIÓN DE RED E IoT (Cambia estos datos con los tuyos)
// ============================================================================
const char* WIFI_SSID     = "MOVISTAR_B640";          // Nombre de tu red Wi-Fi
const char* WIFI_PASSWORD = "vVTjrGtxAnjyRFTaXogu";      // Contraseña de tu Wi-Fi
const char* MQTT_SERVER   = "broker.hivemq.com";     // Servidor MQTT público y gratuito
const int   MQTT_PORT     = 1883;

// Tópicos MQTT donde el ESP32 publicará los datos (Personaliza "grupoXX")
const char* TOPIC_TEMP    = "upc/grupo12/temperatura";
const char* TOPIC_HUM     = "upc/grupo12/humedad";
const char* TOPIC_GAS     = "upc/grupo12/gas";
const char* TOPIC_ALERT   = "upc/grupo12/alerta";

// ============================================================================
// 2. CONFIGURACIÓN DE HARDWARE (OLED SH1106 e I2C Bus compartida)
// ============================================================================
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);
Adafruit_BME680 bme;

// Clientes de red
WiFiClient espClient;
PubSubClient mqttClient(espClient);

// Variables de control de tiempos y umbrales
const float UMBRAL_GAS_CRITICO = 50000.0; // 50k Ohmios
unsigned long tiempoInicio;
const unsigned long TIEMPO_CALENTAMIENTO = 180000; // 3 minutos en milisegundos
unsigned long ultimoEnvioMQTT = 0;
const unsigned long INTERVALO_MQTT = 10000; // Enviar a la nube cada 10 segundos

// ============================================================================
// FNC: Conexión al punto de acceso Wi-Fi
// ============================================================================
void setup_wifi() {
  delay(100);
  Serial.print("\nConectando a la red: ");
  Serial.println(WIFI_SSID);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  // Intentar conectar de fondo mientras informamos en consola
  int intentos = 0;
  while (WiFi.status() != WL_CONNECTED && intentos < 15) {
    delay(500);
    Serial.print(".");
    intentos++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n[Wi-Fi] ¡Conectado con éxito!");
    Serial.print("[Wi-Fi] Dirección IP asignada: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\n[Wi-Fi] Tiempo de espera agotado. El sistema operará en modo offline.");
  }
}

// ============================================================================
// FNC: Reconexión al servidor MQTT
// ============================================================================
void reconectar_mqtt() {
  // Bucle hasta que logremos conectar (máximo 3 intentos para no colgar el bucle local)
  int intentos = 0;
  while (!mqttClient.connected() && intentos < 3) {
    Serial.print("[MQTT] Intentando conexión al broker...");
    // Crear un ID de cliente único basado en la MAC del chip
    String clienteID = "ESP32Client-" + String(random(0, 9999));
    
    if (mqttClient.connect(clienteID.c_str())) {
      Serial.println(" ¡CONECTADO!");
    } else {
      Serial.print(" Falló, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" Reintentando en 2 segundos...");
      delay(2000);
      intentos++;
    }
  }
}

// ============================================================================
// SETUP: Inicialización del sistema
// ============================================================================
void setup() {
  Serial.begin(115200);
  Wire.begin(); 
  u8g2.begin();

  Serial.println("=== SISTEMA DE MONITOREO AMBIENTAL IoT COMPLETO ===");
  
  // Enlace físico del sensor BME680 en 0x77
  while (!bme.begin(0x77)) {
    Serial.println("[ERROR] BME680 ausente en 0x77.");
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_7x14_tr);
    u8g2.drawStr(0, 35, "ERROR SENSOR 0x77");
    u8g2.sendBuffer();
    delay(1000);
  }
  
  bme.setTemperatureOversampling(BME680_OS_8X);
  bme.setHumidityOversampling(BME680_OS_2X);
  bme.setGasHeater(320, 150);

  // Inicializar funciones de comunicaciones
  setup_wifi();
  mqttClient.setServer(MQTT_SERVER, MQTT_PORT);

  tiempoInicio = millis();
}

// ============================================================================
// LOOP: Bucle de ejecución continua
// ============================================================================
void loop() {
  // Asegurar la estabilidad del cliente MQTT si hay Wi-Fi disponible
  if (WiFi.status() == WL_CONNECTED) {
    if (!mqttClient.connected()) {
      reconectar_mqtt();
    }
    mqttClient.loop();
  }

  // Lectura física de magnitudes del sensor
  if (!bme.performReading()) {
    Serial.println("[HARDWARE] Error al capturar datos del BME680.");
    return;
  }

  float temperatura = bme.temperature;
  float humedad = bme.humidity;
  float resistenciaGas = bme.gas_resistance;
  float gasKOhms = resistenciaGas / 1000.0;

  bool calentamientoTerminado = (millis() - tiempoInicio) > TIEMPO_CALENTAMIENTO;
  bool hayAlerta = (resistenciaGas < UMBRAL_GAS_CRITICO) && calentamientoTerminado;

  // --- GESTIÓN DE LA PANTALLA LOCAL (PANTALLA OLED) ---
  u8g2.clearBuffer();
  if (hayAlerta) {
    // Interfaz de alarma visual
    u8g2.setFont(u8g2_font_9x15_tf);
    u8g2.drawStr(0, 18, "P E L I G R O");
    u8g2.drawHLine(0, 22, 128);
    u8g2.setFont(u8g2_font_7x14_tr);
    u8g2.drawStr(0, 42, "¡VENTILAR SALA!");
    u8g2.setCursor(0, 60);
    u8g2.print("Gas: "); u8g2.print(gasKOhms, 1); u8g2.print(" kOhm");
  } else {
    // Interfaz de operación normal
    u8g2.setFont(u8g2_font_6x12_tr);
    u8g2.drawStr(0, 10, calentamientoTerminado ? "MONITOR AMBIENTAL IoT" : "SISTEMA (CALENTANDO)");
    u8g2.drawHLine(0, 13, 128);
    
    u8g2.setFont(u8g2_font_7x14_tr);
    u8g2.setCursor(0, 31); u8g2.print("Temp: "); u8g2.print(temperatura, 1); u8g2.print(" C");
    u8g2.setCursor(0, 47); u8g2.print("Hum:  "); u8g2.print(humedad, 1); u8g2.print(" %");
    u8g2.setCursor(0, 63); u8g2.print("Gas:  "); u8g2.print(gasKOhms, 1); u8g2.print(" kOhm");
  }
  u8g2.sendBuffer();

  // --- TRANSMISIÓN DE DATOS IoT (MQTT) ---
  // Publica telemetría de forma asíncrona cada 10 segundos sin congelar la pantalla
  unsigned long tiempoActual = millis();
  if (tiempoActual - ultimoEnvioMQTT >= INTERVALO_MQTT) {
    ultimoEnvioMQTT = tiempoActual;

    if (WiFi.status() == WL_CONNECTED && mqttClient.connected()) {
      // Convertimos los valores numéricos a cadenas de texto plano para enviarlas por MQTT
      mqttClient.publish(TOPIC_TEMP, String(temperatura, 2).c_str());
      mqttClient.publish(TOPIC_HUM, String(humedad, 2).c_str());
      mqttClient.publish(TOPIC_GAS, String(gasKOhms, 2).c_str());
      
      // Publicamos el estado de alarma
      mqttClient.publish(TOPIC_ALERT, hayAlerta ? "ACTIVADA" : "NORMAL");
      
      Serial.println("[MQTT] Paquete de telemetría enviado con éxito a la nube.");
    } else {
      Serial.println("[MQTT] Envío omitido: Dispositivo sin conexión de red.");
    }
  }

  // Retardo base para refresco local
  delay(2000);
}