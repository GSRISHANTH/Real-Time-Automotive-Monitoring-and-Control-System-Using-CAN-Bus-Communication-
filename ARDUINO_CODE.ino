#include <SPI.h>
#include <mcp_can.h>

// =====================================================
// MCP2515
// =====================================================

#define CAN_CS_PIN 10

// =====================================================
// Buzzer
// =====================================================

#define BUZZER_PIN 9

// =====================================================
// L293D Motor Driver
// =====================================================

#define IN1 3
#define IN2 4
#define ENA 6

#define IN3 7
#define IN4 8
#define ENB 5

// =====================================================
// Settings
// =====================================================

#define MOTOR_SPEED 80

#define OBSTACLE_DIST_CM 10

#define EXPECTED_ID 0x100

// =====================================================

MCP_CAN CAN(CAN_CS_PIN);

// =====================================================

int distance = 255;
int ldr = 1;

unsigned long lastPrint = 0;

// =====================================================
// Motor Functions
// =====================================================

void motorsForward()
{
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);

  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);

  analogWrite(ENA, MOTOR_SPEED);
  analogWrite(ENB, MOTOR_SPEED);
}

// =====================================================

void motorsStop()
{
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);

  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);

  analogWrite(ENA, 0);
  analogWrite(ENB, 0);
}

// =====================================================

void setup()
{
  Serial.begin(115200);

  delay(1000);

  Serial.println("====================================");
  Serial.println("AUTOMOTIVE CAN CONTROL SYSTEM");
  Serial.println("Arduino Receiver ECU Starting...");
  Serial.println("====================================");

  // =================================================
  // Motor Pins
  // =================================================

  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(ENA, OUTPUT);

  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  pinMode(ENB, OUTPUT);

  // =================================================
  // Buzzer
  // =================================================

  pinMode(BUZZER_PIN, OUTPUT);

  digitalWrite(BUZZER_PIN, LOW);

  motorsStop();

  // =================================================
  // CAN INIT
  // =================================================

  Serial.println("Initializing MCP2515...");

  while (CAN.begin(MCP_ANY, CAN_500KBPS, MCP_16MHZ) != CAN_OK)
  {
    Serial.println("CAN Init Failed...");
    delay(1000);
  }

  Serial.println("CAN Init Success");

  CAN.setMode(MCP_NORMAL);

  Serial.println("Arduino Receiver Ready");
  Serial.println("====================================");
}

// =====================================================

void loop()
{
  unsigned long rxId;
  unsigned char len = 0;
  unsigned char buf[8];

  bool newData = false;

  // =================================================
  // RECEIVE CAN DATA
  // =================================================

  if (CAN.checkReceive() == CAN_MSGAVAIL)
  {
    CAN.readMsgBuf(&rxId, &len, buf);

    if (rxId == EXPECTED_ID && len >= 2)
    {
      distance = buf[0];
      ldr = buf[1];

      newData = true;
    }
  }

  // =================================================
  // SAFETY LOGIC
  // =================================================

  bool safeCondition =
      (distance >= OBSTACLE_DIST_CM) &&
      (ldr == 0);

  // =================================================
  // SAFE CONDITION
  // =================================================

  if (safeCondition)
  {
    motorsForward();

    digitalWrite(BUZZER_PIN, LOW);
  }

  // =================================================
  // UNSAFE CONDITION
  // =================================================

  else
  {
    motorsStop();

    digitalWrite(BUZZER_PIN, HIGH);
  }

  // =================================================
  // DETAILED LIVE SERIAL MONITOR
  // =================================================

  if (millis() - lastPrint >= 300)
  {
    lastPrint = millis();

    Serial.println("------------------------------------");

    // CAN Status
    Serial.print("CAN Status      : ");

    if (newData)
      Serial.println("DATA RECEIVED");
    else
      Serial.println("WAITING");

    // Distance
    Serial.print("Distance        : ");
    Serial.print(distance);
    Serial.println(" cm");

    // LDR
    Serial.print("Light Condition : ");

    if (ldr)
      Serial.println("DARK");
    else
      Serial.println("LIGHT");

    // Safety
    Serial.print("System State    : ");

    if (safeCondition)
      Serial.println("SAFE");
    else
      Serial.println("UNSAFE");

    // Motor
    Serial.print("Motor Status    : ");

    if (safeCondition)
      Serial.println("FORWARD");
    else
      Serial.println("STOPPED");

    // Buzzer
    Serial.print("Buzzer Status   : ");

    if (safeCondition)
      Serial.println("OFF");
    else
      Serial.println("ON");

    Serial.println("------------------------------------");
  }
}