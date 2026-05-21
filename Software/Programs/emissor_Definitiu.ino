#include <esp_now.h>
#include <WiFi.h>

uint8_t broadcastAddress[] = {0x8C, 0xBF, 0xEA, 0x8E, 0xDD, 0x6C}; 

const int pinB1 = 4;
const int pinB2 = 5;
const int pinB3 = 6;
const int pinB4 = 43;

typedef struct {
  bool motorA;
  bool motorB;
  bool pinca;
} struct_message;

struct_message dades;
bool estatPinca = false;
unsigned long ultimDebounce = 0;

void setup() {
  Serial.begin(115200);
  pinMode(pinB1, INPUT_PULLUP);
  pinMode(pinB2, INPUT_PULLUP);
  pinMode(pinB3, INPUT_PULLUP);
  pinMode(pinB4, INPUT_PULLUP);
  
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  if (esp_now_init() != ESP_OK) return;
  
  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;
  esp_now_add_peer(&peerInfo);
}

void loop() {
  bool estatB1 = (digitalRead(pinB1) == LOW);
  bool estatB2 = (digitalRead(pinB2) == LOW);

  if (digitalRead(pinB3) == LOW && (millis() - ultimDebounce > 250)) {
    estatPinca = true;
    ultimDebounce = millis();
  }
  
  if (digitalRead(pinB4) == LOW && (millis() - ultimDebounce > 250)) {
    estatPinca = false;
    ultimDebounce = millis();
  }

  if (dades.motorA != estatB1 || dades.motorB != estatB2 || dades.pinca != estatPinca) {
    dades.motorA = estatB1;
    dades.motorB = estatB2;
    dades.pinca = estatPinca;
    esp_now_send(broadcastAddress, (uint8_t *) &dades, sizeof(dades));
    Serial.printf("Enviat -> MotorA:%d | MotorB:%d | Pinca:%d\n", dades.motorA, dades.motorB, dades.pinca);
  }
  delay(10);
}
