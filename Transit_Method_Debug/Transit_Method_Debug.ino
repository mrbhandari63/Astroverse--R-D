#include <Wire.h>
#include <BH1750.h>

BH1750 lightMeter;

// Filter Settings
const int windowSize = 20;     // Smoothing window
float readings[windowSize];    
int readIndex = 0;             
float total = 0;               

void setup() {
  Serial.begin(115200);
  
  // Wait a moment for serial to initialize
  delay(2000); 
  Serial.println("\n--- BH1750 Transit Method Debug ---");
  
  Wire.begin();
  
  // Try initializing the sensor
  Serial.println("Initializing BH1750...");
  
  // High Res Mode 2 provides 0.5 lux resolution
  if (lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE_2)) {
    Serial.println("BH1750 Initialized Successfully!");
    for (int i = 0; i < windowSize; i++) readings[i] = 0;
  } else {
    Serial.println("ERROR: BH1750 initialization failed!");
    Serial.println("Please check:");
    Serial.println("1. Wiring: SDA and SCL pins");
    Serial.println("2. Power: 3.3V or 5V (depending on your module)");
    Serial.println("3. I2C Address (Try running an I2C scanner)");
    while(1) {
      delay(2000);
      Serial.println("Halting. Please fix wiring and reset.");
    }
  }
}

void loop() {
  float rawLux = lightMeter.readLightLevel();

  // Moving Average Filter
  total = total - readings[readIndex];    
  readings[readIndex] = rawLux;           
  total = total + readings[readIndex];    
  readIndex = (readIndex + 1) % windowSize; 
  
  float average = total / windowSize;

  // Print both raw and average for debugging in Serial Monitor/Plotter
  Serial.print("Raw:");
  Serial.print(rawLux, 2);
  Serial.print("\tFiltered:");
  Serial.println(average, 2); 

  delay(100); // Slower delay for readable serial monitor
}
