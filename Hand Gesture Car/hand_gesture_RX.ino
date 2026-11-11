#include <RH_ASK.h>
#include <SPI.h>


// =====================================================
// 433 MHz RECEIVER
// =====================================================

// RX DATA -> Arduino Nano D13

RH_ASK rf_driver(2000, 13);


// =====================================================
// L298N MOTOR PINS
// =====================================================

// LEFT MOTOR

#define ENA 5
#define IN1 2
#define IN2 3


// RIGHT MOTOR

#define ENB 6
#define IN3 4
#define IN4 7


// =====================================================
// CONTROL SETTINGS
// =====================================================

#define DEAD_ZONE 8

#define MAX_TILT 45

#define MIN_SPEED 70


// =====================================================
// RF SAFETY
// =====================================================

unsigned long lastPacketTime = 0;

#define RF_TIMEOUT 500


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


  // Motor pins
  pinMode(ENA, OUTPUT);

  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);

  pinMode(ENB, OUTPUT);

  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);


  stopCar();


  // Start RF
  if (!rf_driver.init())
  {
    Serial.println("ERROR: RF receiver failed!");
    while (1);
  }


  Serial.println();
  Serial.println("================================");
  Serial.println("   433 MHz RX STARTED");
  Serial.println("================================");

  Serial.println("RF DATA PIN: D13");
  Serial.println("Waiting for transmitter...");
  Serial.println();
}


// =====================================================
// LOOP
// =====================================================

void loop()
{
  uint8_t buffer[sizeof(data)];

  uint8_t bufferLength = sizeof(data);


  // ===================================================
  // RECEIVE DATA
  // ===================================================

  if (rf_driver.recv(buffer, &bufferLength))
  {

    if (bufferLength == sizeof(data))
    {

      memcpy(
        &data,
        buffer,
        sizeof(data)
      );


      // Update packet timer
      lastPacketTime = millis();


      // Control car
      controlCar(
        data.pitch,
        data.roll
      );
    }
  }


  // ===================================================
  // RF SIGNAL LOST
  // ===================================================

  if (millis() - lastPacketTime > RF_TIMEOUT)
  {

    stopCar();

    static bool warningShown = false;

    if (!warningShown)
    {
      Serial.println();
      Serial.println("!!! RF SIGNAL LOST - CAR STOPPED !!!");
      Serial.println();

      warningShown = true;
    }
  }
  else
  {
    // Allow warning again after connection returns
    static bool warningShown = false;
    warningShown = false;
  }
}


// =====================================================
// CAR CONTROL
// =====================================================

void controlCar(
  int pitch, 
  int roll
)
{

  // ---------------------------------------------------
  // DEAD ZONE
  // ---------------------------------------------------

  if (abs(pitch) < DEAD_ZONE)
  {
    pitch = 0;
  }

  if (abs(roll) < DEAD_ZONE)
  {
    roll = 0;
  }


  // ---------------------------------------------------
  // STOP
  // ---------------------------------------------------

  if (pitch == 0 && roll == 0)
  {
    stopCar();

    Serial.print("RX ← Pitch: ");
    Serial.print(data.pitch);

    Serial.print(" | Roll: ");
    Serial.print(data.roll);

    Serial.println(" | STOP | L:0 R:0");

    return;
  }


  // ---------------------------------------------------
  // LIMIT TILT
  // ---------------------------------------------------

  pitch = constrain(
    pitch,
    -MAX_TILT,
    MAX_TILT
  );

  roll = constrain(
    roll,
    -MAX_TILT,
    MAX_TILT
  );


  // ---------------------------------------------------
  // SPEED
  // ---------------------------------------------------

  int forwardSpeed =
    map(
      abs(pitch),
      DEAD_ZONE,
      MAX_TILT,
      MIN_SPEED,
      255
    );


  int turnSpeed =
    map(
      abs(roll),
      DEAD_ZONE,
      MAX_TILT,
      MIN_SPEED,
      255
    );


  forwardSpeed =
    constrain(
      forwardSpeed,
      MIN_SPEED,
      255
    );


  turnSpeed =
    constrain(
      turnSpeed,
      MIN_SPEED,
      255
    );


  // ===================================================
  // FORWARD
  // ===================================================

  if (pitch > DEAD_ZONE)
  {

    // FORWARD + RIGHT
    if (roll > DEAD_ZONE)
    {

      int leftMotor = forwardSpeed;

      int rightMotor =
        forwardSpeed - turnSpeed;


      rightMotor =
        constrain(
          rightMotor,
          0,
          255
        );


      driveMotors(
        leftMotor,
        rightMotor
      );


      printStatus(
        "FORWARD + RIGHT",
        leftMotor,
        rightMotor
      );
    }


    // FORWARD + LEFT
    else if (roll < -DEAD_ZONE)
    {

      int leftMotor =
        forwardSpeed - turnSpeed;

      int rightMotor =
        forwardSpeed;


      leftMotor =
        constrain(
          leftMotor,
          0,
          255
        );


      driveMotors(
        leftMotor,
        rightMotor
      );


      printStatus(
        "FORWARD + LEFT",
        leftMotor,
        rightMotor
      );
    }


    // FORWARD
    else
    {

      driveMotors(
        forwardSpeed,
        forwardSpeed
      );


      printStatus(
        "FORWARD",
        forwardSpeed,
        forwardSpeed
      );
    }
  }


  // ===================================================
  // BACKWARD
  // ===================================================

  else if (pitch < -DEAD_ZONE)
  {

    // BACKWARD + RIGHT
    if (roll > DEAD_ZONE)
    {

      int leftMotor =
        -forwardSpeed;

      int rightMotor =
        -(forwardSpeed - turnSpeed);


      driveMotors(
        leftMotor,
        rightMotor
      );


      printStatus(
        "BACKWARD + RIGHT",
        leftMotor,
        rightMotor
      );
    }


    // BACKWARD + LEFT
    else if (roll < -DEAD_ZONE)
    {

      int leftMotor =
        -(forwardSpeed - turnSpeed);

      int rightMotor =
        -forwardSpeed;


      driveMotors(
        leftMotor,
        rightMotor
      );


      printStatus(
        "BACKWARD + LEFT",
        leftMotor,
        rightMotor
      );
    }


    // BACKWARD
    else
    {

      driveMotors(
        -forwardSpeed,
        -forwardSpeed
      );


      printStatus(
        "BACKWARD",
        -forwardSpeed,
        -forwardSpeed
      );
    }
  }


  // ===================================================
  // LEFT / RIGHT
  // ===================================================

  else
  {

    // RIGHT
    if (roll > DEAD_ZONE)
    {

      driveMotors(
        turnSpeed,
        -turnSpeed
      );


      printStatus(
        "RIGHT",
        turnSpeed,
        -turnSpeed
      );
    }


    // LEFT
    else if (roll < -DEAD_ZONE)
    {

      driveMotors(
        -turnSpeed,
        turnSpeed
      );


      printStatus(
        "LEFT",
        -turnSpeed,
        turnSpeed
      );
    }
  }
}


// =====================================================
// SERIAL STATUS
// =====================================================

void printStatus(
  const char *direction,
  int leftMotor,
  int rightMotor
)
{

  Serial.print("RX ← Pitch: ");
  Serial.print(data.pitch);

  Serial.print(" | Roll: ");
  Serial.print(data.roll);

  Serial.print(" | ");
  Serial.print(direction);

  Serial.print(" | L:");
  Serial.print(leftMotor);

  Serial.print(" R:");
  Serial.println(rightMotor);
}


// =====================================================
// MOTOR CONTROL
// =====================================================

void driveMotors(
  int leftSpeed,
  int rightSpeed
)
{

  leftSpeed =
    constrain(
      leftSpeed,
      -255,
      255
    );


  rightSpeed =
    constrain(
      rightSpeed,
      -255,
      255
    );


  // ===================================================
  // LEFT MOTOR
  // ===================================================

  if (leftSpeed > 0)
  {

    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);

    analogWrite(
      ENA,
      leftSpeed
    );
  }

  else if (leftSpeed < 0)
  {

    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);

    analogWrite(
      ENA,
      -leftSpeed
    );
  }

  else
  {

    digitalWrite(IN1, LOW);
    digitalWrite(IN2, LOW);

    analogWrite(
      ENA,
      0
    );
  }


  // ===================================================
  // RIGHT MOTOR
  // ===================================================

  if (rightSpeed > 0)
  {

    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);

    analogWrite(
      ENB,
      rightSpeed
    );
  }

  else if (rightSpeed < 0)
  {

    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);

    analogWrite(
      ENB,
      -rightSpeed
    );
  }

  else
  {

    digitalWrite(IN3, LOW);
    digitalWrite(IN4, LOW);

    analogWrite(
      ENB,
      0
    );
  }
}


// =====================================================
// STOP CAR
// =====================================================

void stopCar()
{

  analogWrite(ENA, 0);

  analogWrite(ENB, 0);


  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);

  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
}
