#include <esp_now.h>
#include <WiFi.h>
#include <ESP32Servo.h>

Servo pincaServo;
const int pinServo = 1;

const int releA = 9;
const int releB = 8;

const int pinSensor = 2;

typedef struct {
  bool motorA;
  bool motorB;
  bool pinca;
} struct_message;

struct_message dadesRebudes;

bool ordreMotorA = false;
bool ordreMotorB = false;

bool estatBloqueigPujada = false;
unsigned long tempsIniciBaixada = 0;
bool baixantActualment = false;

#if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
void alRebre(const esp_now_recv_info_t *info, const uint8_t *data, int len) {
#else
void alRebre(const uint8_t *mac, const uint8_t *data, int len) {
#endif

  memcpy(&dadesRebudes, data, sizeof(dadesRebudes));
  ordreMotorA = dadesRebudes.motorA;
  ordreMotorB = dadesRebudes.motorB;

  if (dadesRebudes.pinca == true) {
    pincaServo.write(70);
  } else {
    pincaServo.write(0);
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(releA, OUTPUT);
  pinMode(releB, OUTPUT);
  pinMode(pinSensor, INPUT_PULLUP);

  digitalWrite(releA, LOW);
  digitalWrite(releB, LOW);

  ESP32PWM::allocateTimer(0);
  pincaServo.setPeriodHertz(50);
  pincaServo.attach(pinServo, 500, 2400);

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  if (esp_now_init() != ESP_OK) return;

  esp_now_register_recv_cb(alRebre);
}

void loop() {
  bool imantDetectat = (digitalRead(pinSensor) == LOW);

  if (imantDetectat) {
    estatBloqueigPujada = true;
  }

  if (ordreMotorB) {
    if (!baixantActualment) {
      baixantActualment = true;
      tempsIniciBaixada = millis();
    } else {
      if (millis() - tempsIniciBaixada >= 2000) {
        estatBloqueigPujada = false;
      }
    }
  } else {
    baixantActualment = false;
  }

  if (estatBloqueigPujada) {
    digitalWrite(releA, LOW);
    digitalWrite(releB, ordreMotorB ? HIGH : LOW);
  } else {
    digitalWrite(releA, ordreMotorA ? HIGH : LOW);
    digitalWrite(releB, ordreMotorB ? HIGH : LOW);
  }

  delay(10);
}