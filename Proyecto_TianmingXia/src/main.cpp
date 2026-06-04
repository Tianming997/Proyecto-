#include <Arduino.h>
#include <Wire.h>
#include <U8g2lib.h>
#include "Adafruit_BME680.h"

// 1. Pantalla OLED en pines estándar (21 y 22)
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

// 2. Sensor BME680 en bus secundario
Adafruit_BME680 bme;
TwoWire I2C_Sensor = TwoWire(1); // Canal 1 para pines 33 y 32

void setup() {
  Serial.begin(115200);

  // Despertar al sensor forzando el modo I2C
  pinMode(25, OUTPUT);
  digitalWrite(25, HIGH); 
  delay(500);

  // Iniciar pantalla y bus del sensor
  u8g2.begin();
  I2C_Sensor.begin(33, 32); 

  Serial.println("Buscando el sensor BME680...");
  bool conectado = false;
  
  // BUCLE DE BÚSQUEDA INFINITO
  while (!conectado) {
    if (bme.begin(0x77, &I2C_Sensor)) {
      Serial.println("¡BINGO! Conectado en 0x77.");
      conectado = true;
    } else if (bme.begin(0x76, &I2C_Sensor)) {
      Serial.println("¡BINGO! Conectado en 0x76.");
      conectado = true;
    } else {
      Serial.println("Sensor suelto. Aprieta pines 33, 32 y 25...");
      
      // Aviso visual en la pantalla
      u8g2.clearBuffer();
      u8g2.setFont(u8g2_font_6x10_tf);
      u8g2.drawStr(0, 30, "APRIETA CABLES!");
      u8g2.sendBuffer();
      
      delay(1000); 
    }
  }
  
  // Si sale del bucle, configuramos la precisión
  bme.setTemperatureOversampling(BME680_OS_8X);
  bme.setHumidityOversampling(BME680_OS_2X);
  bme.setGasHeater(320, 150); 
}

void loop() {
  // Intentar leer el sensor
  if (!bme.performReading()) {
    Serial.println("Microcorte detectado... intentando recuperar.");
    return; // Salta esta vuelta y lo vuelve a intentar
  }

  // --- DIBUJAR EN PANTALLA ---
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

  // --- IMPRIMIR EN MONITOR SERIAL ---
  Serial.print("Temperatura real: "); Serial.print(bme.temperature);
  Serial.print(" C | Gas: "); Serial.println(bme.gas_resistance);

  delay(2000); // Refresca cada 2 segundos
}