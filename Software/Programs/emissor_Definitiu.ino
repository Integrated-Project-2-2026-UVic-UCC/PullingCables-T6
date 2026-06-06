/**
 * PROJECT: Drone Cable Routing Accessory
 * MODULE: Transmitter (Remote Control)
 * DESCRIPTION: Reads the state of 4 pushbuttons and sends movement 
 * and gripper control commands to the receiver via the ESP-NOW protocol.
 */

#include <esp_now.h>
#include <WiFi.h>

// --- NETWORK CONFIGURATION ---
// MAC address of the receiver board. Must be updated if the board changes.
uint8_t broadcastAddress[] = {0x8C, 0xBF, 0xEA, 0x8E, 0xDD, 0x6C}; 

// --- PIN DEFINITION ---
const int pinB1 = 4;   // Pushbutton 1: Reel in cable (Up)
const int pinB2 = 5;   // Pushbutton 2: Reel out cable (Down)
const int pinB3 = 6;   // Pushbutton 3: Open gripper
const int pinB4 = 43;  // Pushbutton 4: Close gripper

// --- DATA STRUCTURE ---
// The data packet structure must match exactly on both transmitter and receiver
typedef struct {
  bool motorA; // Upward relay state
  bool motorB; // Downward relay state
  bool pinca;  // Gripper servo state
} struct_message;

struct_message dades;
bool estatPinca = false;
unsigned long ultimDebounce = 0; // Variable for software debounce control

void setup() {
  Serial.begin(115200);

  // Configure button pins with internal pull-up resistors
  // Default state is HIGH; pressing the button pulls it LOW
  pinMode(pinB1, INPUT_PULLUP);
  pinMode(pinB2, INPUT_PULLUP);
  pinMode(pinB3, INPUT_PULLUP);
  pinMode(pinB4, INPUT_PULLUP);
  
  // Configure Wi-Fi in Station mode (required for ESP-NOW)
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  // Initialize the ESP-NOW protocol
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  
  // Register the receiver device (Peer)
  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;
  esp_now_add_peer(&peerInfo);
}

void loop() {
  // Read the state of the continuous movement buttons
  bool estatB1 = (digitalRead(pinB1) == LOW);
  bool estatB2 = (digitalRead(pinB2) == LOW);

  // Gripper control with debounce logic (250 ms margin)
  // Ensures a single short press toggles the state without reading multiple inputs
  if (digitalRead(pinB3) == LOW && (millis() - ultimDebounce > 250)) {
    estatPinca = true;
    ultimDebounce = millis();
  }
  
  if (digitalRead(pinB4) == LOW && (millis() - ultimDebounce > 250)) {
    estatPinca = false;
    ultimDebounce = millis();
  }

  // Only transmit data if there is a state change in any button
  // This saves battery life and prevents radio spectrum saturation
  if (dades.motorA != estatB1 || dades.motorB != estatB2 || dades.pinca != estatPinca) {
    dades.motorA = estatB1;
    dades.motorB = estatB2;
    dades.pinca = estatPinca;
    
    // Send the data packet to the receiver
    esp_now_send(broadcastAddress, (uint8_t *) &dades, sizeof(dades));
    Serial.printf("Sent -> MotorA:%d | MotorB:%d | Gripper:%d\n", dades.motorA, dades.motorB, dades.pinca);
  }
  
  // Short delay for microcontroller stability
  delay(10);
}