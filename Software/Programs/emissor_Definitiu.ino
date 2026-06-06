/**
 * PROJECTE: Sistema de politja i ganxo per a dron
 * MÒDUL: Emissor (Comandament a distància)
 * DESCRIPCIÓ: Llegeix l'estat de 4 polsadors i envia les ordres de moviment 
 * i control de la pinça al receptor mitjançant el protocol ESP-NOW.
 */

#include <esp_now.h>
#include <WiFi.h>

// --- CONFIGURACIÓ DE XARXA ---
// Adreça MAC de la placa receptora. S'ha de canviar si es canvia de placa.
uint8_t broadcastAddress[] = {0x8C, 0xBF, 0xEA, 0x8E, 0xDD, 0x6C}; 

// --- DEFINICIÓ DE PINS ---
const int pinB1 = 4;   // Polsador 1: Pujar cable (Enrotllar)
const int pinB2 = 5;   // Polsador 2: Baixar cable (Desenrotllar)
const int pinB3 = 6;   // Polsador 3: Obrir pinça
const int pinB4 = 43;  // Polsador 4: Tancar pinça

// --- ESTRUCTURA DE DADES ---
// El paquet de dades ha de ser idèntic a l'emissor i al receptor
typedef struct {
  bool motorA; // Estat del relé de pujada
  bool motorB; // Estat del relé de baixada
  bool pinca;  // Estat del servomotor de la pinça
} struct_message;

struct_message dades;
bool estatPinca = false;
unsigned long ultimDebounce = 0; // Variable per al control d'antirebots (debounce)

void setup() {
  Serial.begin(115200);

  // Configurem els pins dels botons amb resistència pull-up interna
  // L'estat per defecte serà HIGH, i en prémer el botó serà LOW
  pinMode(pinB1, INPUT_PULLUP);
  pinMode(pinB2, INPUT_PULLUP);
  pinMode(pinB3, INPUT_PULLUP);
  pinMode(pinB4, INPUT_PULLUP);
  
  // Configurem el Wi-Fi en mode Estació (necessari per a ESP-NOW)
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  // Inicialitzem el protocol ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error inicialitzant ESP-NOW");
    return;
  }
  
  // Registrem el dispositiu receptor (Peer)
  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;
  esp_now_add_peer(&peerInfo);
}

void loop() {
  // Llegeix l'estat dels botons de moviment continus
  bool estatB1 = (digitalRead(pinB1) == LOW);
  bool estatB2 = (digitalRead(pinB2) == LOW);

  // Control de la pinça amb antirebots (250 ms de marge)
  // Permet que una sola pulsació curta canviï l'estat sense fer lectures dobles
  if (digitalRead(pinB3) == LOW && (millis() - ultimDebounce > 250)) {
    estatPinca = true;
    ultimDebounce = millis();
  }
  
  if (digitalRead(pinB4) == LOW && (millis() - ultimDebounce > 250)) {
    estatPinca = false;
    ultimDebounce = millis();
  }

  // Només enviem dades si hi ha hagut algun canvi en l'estat dels botons
  // Això estalvia bateria i no satura la freqüència de ràdio
  if (dades.motorA != estatB1 || dades.motorB != estatB2 || dades.pinca != estatPinca) {
    dades.motorA = estatB1;
    dades.motorB = estatB2;
    dades.pinca = estatPinca;
    
    // Enviem el paquet de dades cap al receptor
    esp_now_send(broadcastAddress, (uint8_t *) &dades, sizeof(dades));
    Serial.printf("Enviat -> MotorA:%d | MotorB:%d | Pinca:%d\n", dades.motorA, dades.motorB, dades.pinca);
  }
  
  // Petita pausa per estabilitat del microcontrolador
  delay(10);
}