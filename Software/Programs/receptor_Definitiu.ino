/**
 * PROJECT: Drone Cable Routing Accessory
 * MODULE: Receiver (Integrated in the drone housing)
 * DESCRIPTION: Receives commands via ESP-NOW and controls an H-bridge via
 * relays for the main motor, alongside a servo motor for the gripper.
 * Includes a safety interlock logic with a magnetic limit switch.
 */

#include <esp_now.h>
#include <WiFi.h>
#include <ESP32Servo.h>

// --- COMPONENT CONFIGURATION ---
Servo pincaServo;
const int pinServo = 1;  // Signal pin for the servo motor (Gripper)

const int releA = 9;     // Pin for Relay A (Reel in / Up)
const int releB = 8;     // Pin for Relay B (Reel out / Down)

const int pinSensor = 2; // Pin for the Magnetic Sensor (Limit switch)

// --- DATA STRUCTURE ---
typedef struct {
  bool motorA;
  bool motorB;
  bool pinca;
} struct_message;

struct_message dadesRebudes;

// --- STATE VARIABLES ---
bool ordreMotorA = false;
bool ordreMotorB = false;

// Variables to manage the safety interlock system
bool estatBloqueigPujada = false;
unsigned long tempsIniciBaixada = 0;
bool baixantActualment = false;

// --- DATA RECEPTION CALLBACK ---
// Compilation directive to maintain compatibility with ESP32 core v2 and v3
#if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
void alRebre(const esp_now_recv_info_t *info, const uint8_t *data, int len) {
#else
void alRebre(const uint8_t *mac, const uint8_t *data, int len) {
#endif

  // Copy the received information into the local data structure
  memcpy(&dadesRebudes, data, sizeof(dadesRebudes));
  ordreMotorA = dadesRebudes.motorA;
  ordreMotorB = dadesRebudes.motorB;

  // Gripper servo motor actuation
  if (dadesRebudes.pinca == true) {
    pincaServo.write(70); // Opening angle of the gripper
  } else {
    pincaServo.write(0);  // Closing angle of the gripper
  }
}

void setup() {
  Serial.begin(115200);

  // Configuration of input and output pins
  pinMode(releA, OUTPUT);
  pinMode(releB, OUTPUT);
  pinMode(pinSensor, INPUT_PULLUP); // Sensor shorts to GND, so PULLUP is used

  // Ensure motors are completely stopped at startup for safety
  digitalWrite(releA, LOW);
  digitalWrite(releB, LOW);

  // Servo motor configuration (Specific frequency and pulses for ESP32)
  ESP32PWM::allocateTimer(0);
  pincaServo.setPeriodHertz(50);
  pincaServo.attach(pinServo, 500, 2400);

  // Initialize the radio in Station mode
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Bind the callback function that executes automatically when data arrives
  esp_now_register_recv_cb(alRebre);
}

void loop() {
  // 1. LIMIT SWITCH READING
  // If the magnet approaches the sensor, the pin drops to LOW triggering the safety lock
  bool imantDetectat = (digitalRead(pinSensor) == LOW);

  if (imantDetectat) {
    estatBloqueigPujada = true;
  }

  // 2. UNLOCK MANAGEMENT (2-second release clearance)
  // If the down command is active, calculate how long the button is held.
  // This prevents the cable from getting stuck at the limit and enforces a safe physical margin.
  if (ordreMotorB) {
    if (!baixantActualment) {
      // Start the downward release timer
      baixantActualment = true;
      tempsIniciBaixada = millis();
    } else {
      // Check if the required 2 seconds of uninterrupted downward movement are met
      if (millis() - tempsIniciBaixada >= 2000) {
        estatBloqueigPujada = false; // Release the safety lock
      }
    }
  } else {
    // If the user releases the button before 2 seconds, reset the timer
    baixantActualment = false;
  }

  // 3. H-BRIDGE UPDATE (RELAYS)
  if (estatBloqueigPujada) {
    // Emergency mode: Block upward movement (Relay A) but allow downward movement (Relay B)
    digitalWrite(releA, LOW);
    digitalWrite(releB, ordreMotorB ? HIGH : LOW);
  } else {
    // Normal mode: Directly apply the commands arriving from the transmitter
    digitalWrite(releA, ordreMotorA ? HIGH : LOW);
    digitalWrite(releB, ordreMotorB ? HIGH : LOW);
  }

  delay(10);
}