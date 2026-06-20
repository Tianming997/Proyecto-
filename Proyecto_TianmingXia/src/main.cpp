#include <Arduino.h>
#include <Wire.h>
#include <U8g2lib.h>
#include "Adafruit_BME680.h"

// 1. Configurar la pantalla en los pines estándar (21 y 22)
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

// 2. Configurar el sensor BME680
Adafruit_BME680 bme; 

void setup() {
  Serial.begin(115200);

  // Arrancar la pantalla y el bus de datos
  u8g2.begin();
  Wire.begin(); 

  Serial.println("Iniciando Sistema Oficial...");
  
  // BUCLE DE SEGURIDAD: Busca el sensor en 0x77
  while (!bme.begin(0x77)) {
    Serial.println("BME680 no responde en 0x77. Comprobando...");
    
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_6x10_tf);
    u8g2.drawStr(0, 30, "BUSCANDO SENSOR...");
    u8g2.sendBuffer();
    delay(1000);
  }
  
  Serial.println("¡TODO CONECTADO! Arrancando motores...");

  // Ajustes de precisión para el BME680
  bme.setTemperatureOversampling(BME680_OS_8X);
  bme.setHumidityOversampling(BME680_OS_2X);
  bme.setGasHeater(320, 150); // 320°C durante 150ms para medir el gas
}

void loop() {
  // Leer los datos del sensor físico
  if (!bme.performReading()) {
    Serial.println("Error de lectura en esta vuelta.");
    return;
  }

  // --- ENVIAR DATOS A LA PANTALLA OLED ---
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_7x14_tr);
  
  u8g2.drawStr(0, 12, "MONITOR DE AIRE");
  u8g2.drawHLine(0, 15, 128);
  
  u8g2.setCursor(0, 35);
  u8g2.print("Temp: "); u8g2.print(bme.temperature, 1); u8g2.print(" C");
  
  u8g2.setCursor(0, 50);
  u8g2.print("Hum:  "); u8g2.print(bme.humidity, 1); u8g2.print(" %");
  
  u8g2.setCursor(0, 64);
  u8g2.print("Gas:  "); u8g2.print(bme.gas_resistance / 1000.0, 1); u8g2.print(" K");
  
  u8g2.sendBuffer();

  // --- ENVIAR DATOS AL MONITOR SERIAL (PC) ---
  Serial.print("Temperatura: "); Serial.print(bme.temperature);
  Serial.print(" C | Humedad: "); Serial.print(bme.humidity);
  Serial.print(" % | Gas: "); Serial.println(bme.gas_resistance / 1000.0);

  delay(2000); // Mediciones cada 2 segundos
}