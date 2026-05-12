#include <SPI.h>
#include <mcp_can.h>

// =====================================================
// MCP2515 CAN Pins
// =====================================================

#define CAN_CS_PIN 5

// =====================================================
// HC-SR04
// =====================================================

#define TRIG_PIN 17
#define ECHO_PIN 16

// =====================================================
// LDR
// =====================================================

#define LDR_PIN 22

// =====================================================

#define CAN_ID 0x100

MCP_CAN CAN(CAN_CS_PIN);

// =====================================================
// Filtered Distance Measurement
// =====================================================

long measureDistance()
{
  long total = 0;

  for (int i = 0; i < 5; i++)
  {
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);

    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);

    digitalWrite(TRIG_PIN, LOW);

    long duration = pulseIn(ECHO_PIN, HIGH, 30000);

    long distance;

    if (duration == 0)
      distance = 255;
    else
      distance = duration * 0.0343 / 2;

    total += distance;

    delay(10);
  }

  long avgDistance = total / 5;

  if (avgDistance > 255)
    avgDistance = 255;

  if (avgDistance < 0)
    avgDistance = 0;

  return avgDistance;
}

// =====================================================

void setup()
{
  Serial.begin(115200);

  delay(1000);

  Serial.println("====================================");
  Serial.println("ESP32 SENSOR ECU");
  Serial.println("Initializing...");
  Serial.println("====================================");

  // =================================================
  // Sensor Pins
  // =================================================

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  pinMode(LDR_PIN, INPUT);

  // =================================================
  // MCP2515 INIT
  // =================================================

  Serial.println("Initializing MCP2515...");

  while (CAN.begin(MCP_ANY, CAN_500KBPS, MCP_16MHZ) != CAN_OK)
  {
    Serial.println("CAN Init Failed...");
    delay(1000);
  }

  Serial.println("CAN Init Success");

  CAN.setMode(MCP_NORMAL);

  Serial.println("ESP32 CAN Sender Ready");
  Serial.println("====================================");
}

// =====================================================

void loop()
{
  long distance = measureDistance();

  // 1 = DARK
  // 0 = LIGHT
  int ldr = digitalRead(LDR_PIN) ? 1 : 0;

  // =================================================
  // Build CAN Frame
  // =================================================

  byte txData[2];

  txData[0] = distance;
  txData[1] = ldr;

  // =================================================
  // Send CAN Frame
  // =================================================

  byte result = CAN.sendMsgBuf(CAN_ID, 0, 2, txData);

  // =================================================
  // Detailed Serial Monitor
  // =================================================

  Serial.println("------------------------------------");

  Serial.print("Distance        : ");
  Serial.print(distance);
  Serial.println(" cm");

  Serial.print("Light Condition : ");

  if (ldr)
    Serial.println("DARK");
  else
    Serial.println("LIGHT");

  Serial.print("CAN Status      : ");

  if (result == CAN_OK)
    Serial.println("TRANSMITTED");
  else
    Serial.println("TX ERROR");

  Serial.println("------------------------------------");

  delay(300);
}