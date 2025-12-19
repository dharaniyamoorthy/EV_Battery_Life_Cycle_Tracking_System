#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_INA219.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>

// PIN definitions
#define ONE_WIRE_BUS 17
#define CURRENT_MOCK_PIN 34 // Potentiometer on GPIO 34 (ADC1_CH6) to mock current (mA)

// --- OLED DISPLAY DEFINITIONS ---
#define SCREEN_WIDTH 128    // OLED display width, in pixels
#define SCREEN_HEIGHT 64    // OLED display height, in pixels
#define OLED_RESET -1       // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< I2C address for 128x64 display (can be 0x3C or 0x3D)

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
//-----------------------------------

// Declare INA219, OneWire, and Temperature objects (Needed for types, even if mocked)
Adafruit_INA219 ina219; 
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

// --- COULOMB COUNTING & CYCLE COUNTING PARAMETERS ---
// TEMPORARY CHANGE FOR QUICK CYCLE DEMONSTRATION
const float BATTERY_CAPACITY_MAH = 250.0; // Reduced to 500mAh // Example: 20Ah Battery for simulation
float accumulated_discharge_mAh = 0.0;      // Tracks mAh removed since the last 'full' state

// --- NEW VARIABLES FOR DETAILED LOGGING ---
float total_discharged_mAh = 0.0; // Total mAh discharged since start of simulation
float total_recharged_mAh = 0.0;  // Total mAh recharged since start of simulation

long battery_cycle_count = 0;              // Total number of full cycles completed

const long SAMPLE_INTERVAL_MS = 1000;       // Sample every 1 second
unsigned long previousMillis = 0;

// Function to simulate voltage (Mock data - drops as charge is removed)
float mock_getBusVoltage_V() {
  float max_v = 12.6;
  float min_v = 11.0;
  // Use a ratio (Discharged mAh / Total Capacity) to determine voltage drop
  float discharge_ratio = accumulated_discharge_mAh / BATTERY_CAPACITY_MAH;
  
  if (discharge_ratio >= 1.0) return min_v; // Fully discharged
  
  // Voltage increases if accumulated_discharge_mAh is negative, but we clip it at 0 in loop()
  return max_v - ( (max_v - min_v) * discharge_ratio );
} 

/**
 * @brief Mocks current flow (Charge/Discharge) based on Potentiometer position.
 * The midpoint of the ADC range (approx. 2048) acts as the switch.
 * - Lower Half: Discharge (Negative Current)
 * - Upper Half: Charge (Positive Current)
 */
float mock_getCurrent_mA() {
  // Read Potentiometer (0 to 4095)
  int sensorValue = analogRead(CURRENT_MOCK_PIN);
  float current_mA;

  if (sensorValue < 2048) {
    // DISCHARGE (Potentiometer in the lower half)
    // Map 0-2047 to -20000mA (heavy load) to -500mA (light load)
    current_mA = map(sensorValue, 0, 2047, -20000, -500);
  } else {
    // CHARGE (Potentiometer in the upper half)
    // Map 2048-4095 to +1000mA (slow charge) to +5000mA (fast charge)
    current_mA = map(sensorValue, 2048, 4095, 1000, 5000);
  }
  
  return current_mA;
}

void setup() {
  Serial.begin(115200);
  Wire.begin(); 
  sensors.begin(); 
  
  // --- OLED Initialization ---
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  display.display(); 
  delay(2000); 
  display.clearDisplay(); 

  Serial.println("INA219 Mock Simulation Started for EV Cycle Counting. Use Potentiometer to switch between Discharge (-) and Charge (+).");
}

void loop() {
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= SAMPLE_INTERVAL_MS) {
    float deltaTime_hours = (float)SAMPLE_INTERVAL_MS / 3600000.0; 

    // --- 1. MOCK/READ SENSOR DATA ---
    float current_mA = mock_getCurrent_mA();
    float voltage_V = mock_getBusVoltage_V();
    
    sensors.requestTemperatures(); 
    float temperature_C = sensors.getTempCByIndex(0);

    // --- 2. COULOMB COUNTING ---
    float charge_transferred_mAh = current_mA * deltaTime_hours;

    if (current_mA < 0) {
      // DISCHARGING
      float discharge_amount = abs(charge_transferred_mAh);
      accumulated_discharge_mAh += discharge_amount;
      total_discharged_mAh += discharge_amount;
    } else if (current_mA > 0) {
      // CHARGING
      float charge_amount = abs(charge_transferred_mAh);
      accumulated_discharge_mAh -= charge_amount;
      total_recharged_mAh += charge_amount;

      if (accumulated_discharge_mAh < 0) {
        accumulated_discharge_mAh = 0; // clip at full charge
      }
    }

    // --- 3. Equivalent Full Cycle Counting (WHOLE numbers only) ---
    long expected_cycle_count = (long)(total_discharged_mAh / BATTERY_CAPACITY_MAH);

    if (expected_cycle_count > battery_cycle_count) {
      battery_cycle_count = expected_cycle_count;
      Serial.println(">>> EV Cycle incremented! <<<");
    }

    // --- 4. DISPLAY ON OLED AND SERIAL ---
    float percentage_remaining = 100.0 * (1.0 - (accumulated_discharge_mAh / BATTERY_CAPACITY_MAH));

    display.clearDisplay(); 
    display.setTextColor(SSD1306_WHITE);

    // Line 1: State of Charge
    display.setTextSize(2); 
    display.setCursor(0, 0);
    display.print("SoC: ");
    display.print(percentage_remaining, 0); 
    display.print("%");

    // Line 2: Voltage & Current
    display.setTextSize(1);
    display.setCursor(0, 20);
    display.print("V: "); display.print(voltage_V, 2); display.print("V");
    display.setCursor(70, 20);
    display.print("I: "); display.print(current_mA / 1000.0, 1); display.print("A"); 

    // Line 3–4: Cycle Count
    display.setCursor(0, 35);
    display.setTextSize(2);
    display.print("Cycles:");
    display.setCursor(0, 50);
    display.setTextSize(2);
    display.print(battery_cycle_count);
    
    display.display(); 
    
    // --- Serial Monitor ---
    Serial.print("V: "); Serial.print(voltage_V, 2); 
    Serial.print("V | I (Mock): "); Serial.print(current_mA, 0); 
    Serial.print("mA | SoC (Mock): "); Serial.print(percentage_remaining, 1); Serial.print("%");
    
    Serial.print(" | Net Discharge: "); Serial.print(accumulated_discharge_mAh, 0); 
    Serial.print("mAh | Total Discharged: "); Serial.print(total_discharged_mAh, 0); 
    Serial.print("mAh | Total Recharged: "); Serial.print(total_recharged_mAh, 0); 
    
    Serial.print("mAh | **EV Cycles: "); Serial.print(battery_cycle_count); Serial.println("**");

    previousMillis = currentMillis;
  }
}
