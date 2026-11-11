#include <Wire.h>
#include <RH_ASK.h>
#include <SPI.h>

// =====================================================
// 433 MHz TRANSMITTER
// =====================================================

// TX DATA -> Arduino Nano D12
RH_ASK rf_driver(2000, 11, 12, 10);


// =====================================================
// MPU6050
// =====================================================

#define MPU_ADDR 0x68

int16_t AcX, AcY, AcZ;
int16_t GyX, GyY, GyZ;


// =====================================================
// CALIBRATION
// =====================================================

float pitchOffset = 0;
float rollOffset = 0;


// =====================================================
// DATA PACKET
// =====================================================

struct ControlData    
{
  int16_t pitch;
  int16_t roll;
};

ControlData data;


// =====================================================
// SETUP
// =====================================================

void setup()
{
  Serial.begin(9600);

  Wire.begin();


  // Wake MPU6050
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x6B);
  Wire.write(0x00);
  Wire.endTransmission(true);


  // Start RF
  if (!rf_driver.init())
  {
    Serial.println("ERROR: RF transmitter failed!");
    while (1);
  }


  Serial.println();
  Serial.println("================================");
  Serial.println("   433 MHz TX STARTED");
  Serial.println("================================");


  // MPU calibration
  calibrateMPU();


  Serial.println("TX READY");
  Serial.println();
}


// =====================================================
// LOOP
// =====================================================

void loop()
{
  readMPU();


  // ---------------------------------------------------
  // CALCULATE PITCH
  // ---------------------------------------------------

  float pitch =
    atan2(
      AcY,
      sqrt(
        (float)AcX * AcX +
        (float)AcZ * AcZ
      )
    )
    * 180.0 / PI;


  // ---------------------------------------------------
  // CALCULATE ROLL
  // ---------------------------------------------------

  float roll =
    atan2(
      -AcX,
      AcZ
    )
    * 180.0 / PI;


  // Remove calibration
  pitch -= pitchOffset;
  roll -= rollOffset;


  // Limit angles
  pitch = constrain(pitch, -90, 90);
  roll  = constrain(roll, -90, 90);


  // Store packet
  data.pitch = (int16_t)pitch;
  data.roll  = (int16_t)roll;


  // ===================================================
  // SEND DATA
  // ===================================================

  bool success = rf_driver.send(
    (uint8_t *)&data,
    sizeof(data)
  );

  rf_driver.waitPacketSent();


  // ===================================================
  // SERIAL MONITOR
  // ===================================================

  Serial.print("TX → Pitch: ");
  Serial.print(data.pitch);

  Serial.print("° | Roll: ");
  Serial.print(data.roll);

  Serial.print("° | RF: ");

  if (success)
  {
    Serial.println("SENT");
  }
  else
  {
    Serial.println("FAILED");
  }


  delay(50);
}


// =====================================================
// READ MPU6050
// =====================================================

void readMPU()
{
  Wire.beginTransmission(MPU_ADDR);

  Wire.write(0x3B);

  Wire.endTransmission(false);

  Wire.requestFrom(MPU_ADDR, 14, true);


  AcX = Wire.read() << 8 | Wire.read();

  AcY = Wire.read() << 8 | Wire.read();

  AcZ = Wire.read() << 8 | Wire.read();


  // Temperature
  Wire.read();
  Wire.read();


  GyX = Wire.read() << 8 | Wire.read();

  GyY = Wire.read() << 8 | Wire.read();

  GyZ = Wire.read() << 8 | Wire.read();
}


// =====================================================
// CALIBRATION
// =====================================================

void calibrateMPU()
{
  float pitchSum = 0;
  float rollSum = 0;


  Serial.println();
  Serial.println("Keep MPU6050 LEVEL...");
  Serial.println("Calibration starting...");


  delay(2000);


  for (int i = 0; i < 100; i++)
  {
    readMPU();


    float pitch =
      atan2(
        AcY,
        sqrt(
          (float)AcX * AcX +
          (float)AcZ * AcZ
        )
      )
      * 180.0 / PI;


    float roll =
      atan2(
        -AcX,
        AcZ
      )
      * 180.0 / PI;


    pitchSum += pitch;
    rollSum += roll;


    delay(10);
  }


  pitchOffset = pitchSum / 100.0;
  rollOffset  = rollSum / 100.0;


  Serial.println("Calibration complete.");
  Serial.println();
}