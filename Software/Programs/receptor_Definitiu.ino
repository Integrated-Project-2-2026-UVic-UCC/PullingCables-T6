/**
 * PROJECTE: Sistema de politja i ganxo per a dron
 * MÒDUL: Receptor (Integrat al xassís del dron)
 * DESCRIPCIÓ: Rep ordres de l'emissor via ESP-NOW i controla un pont en H amb
 * relés per moure el motor principal, a més d'un servomotor per a la pinça.
 * Inclou lògica d'enclavament de seguretat amb final de carrera magnètic.
 */

#include <esp_now.h>
#include <WiFi.h>
#include <ESP32Servo.h>

// --- CONFIGURACIÓ DE COMPONENTS ---
Servo pincaServo;
const int pinServo = 1;  // Pin de senyal per al servomotor (Pinça)

const int releA = 9;     // Pin per al Relé A (Pujar/Enrotllar cable)
const int releB = 8;     // Pin per al Relé B (Baixar/Desenrotllar cable)

const int pinSensor = 2; // Pin per al Sensor Magnètic (Final de carrera)

// --- ESTRUCTURA DE DADES ---
typedef struct {
  bool motorA;
  bool motorB;
  bool pinca;
} struct_message;

struct_message dadesRebudes;

// --- VARIABLES D'ESTAT ---
bool ordreMotorA = false;
bool ordreMotorB = false;

// Variables per gestionar l'enclavament de seguretat (Safety Interlock)
bool estatBloqueigPujada = false;
unsigned long tempsIniciBaixada = 0;
bool baixantActualment = false;

// --- FUNCIÓ DE RECEPCIÓ DE DADES ---
// Directiva de compilació per mantenir compatibilitat amb versions v2 i v3 del core ESP32
#if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
void alRebre(const esp_now_recv_info_t *info, const uint8_t *data, int len) {
#else
void alRebre(const uint8_t *mac, const uint8_t *data, int len) {
#endif

  // Copiem la informació rebuda a la nostra estructura de dades
  memcpy(&dadesRebudes, data, sizeof(dadesRebudes));
  ordreMotorA = dadesRebudes.motorA;
  ordreMotorB = dadesRebudes.motorB;

  // Accionament del servomotor de la pinça
  if (dadesRebudes.pinca == true) {
    pincaServo.write(70); // Grau d'obertura de la pinça
  } else {
    pincaServo.write(0);  // Grau de tancament de la pinça
  }
}

void setup() {
  Serial.begin(115200);

  // Configuració de pins de sortida i entrada
  pinMode(releA, OUTPUT);
  pinMode(releB, OUTPUT);
  pinMode(pinSensor, INPUT_PULLUP); // El sensor tanca a GND, per tant usem PULLUP

  // Garantim que els motors estiguin aturats a l'inici per seguretat
  digitalWrite(releA, LOW);
  digitalWrite(releB, LOW);

  // Configuració del servomotor (Freqüència i polsos específics per a l'ESP32)
  ESP32PWM::allocateTimer(0);
  pincaServo.setPeriodHertz(50);
  pincaServo.attach(pinServo, 500, 2400);

  // Inicialització de la ràdio en mode Estació
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error inicialitzant ESP-NOW");
    return;
  }

  // Vinculem la funció que s'executarà automàticament cada cop que arribin dades
  esp_now_register_recv_cb(alRebre);
}

void loop() {
  // 1. LECTURA DEL FINAL DE CARRERA
  // Si l'imant s'apropa al sensor, el pin cau a LOW i s'activa el bloqueig de seguretat
  bool imantDetectat = (digitalRead(pinSensor) == LOW);

  if (imantDetectat) {
    estatBloqueigPujada = true;
  }

  // 2. GESTIÓ DEL DESBLOQUEIG (Alliberament de 2 segons)
  // Si estem demanant baixar, calculem quant de temps mantenim el botó premut.
  // Això evita que el cable es quedi encallat al límit i obliga a crear un marge físic segur.
  if (ordreMotorB) {
    if (!baixantActualment) {
      // Iniciem el cronòmetre de baixada
      baixantActualment = true;
      tempsIniciBaixada = millis();
    } else {
      // Comprovem si ja hem superat els 2 segons de baixada ininterrompuda
      if (millis() - tempsIniciBaixada >= 2000) {
        estatBloqueigPujada = false; // Lliurem el bloqueig
      }
    }
  } else {
    // Si l'usuari deixa anar el botó abans de 2 segons, reiniciem el comptador
    baixantActualment = false;
  }

  // 3. ACTUALITZACIÓ DEL PONT EN H (RELÉS)
  if (estatBloqueigPujada) {
    // Mode d'emergència: Bloquegem la pujada (Relé A) però permetem la baixada (Relé B)
    digitalWrite(releA, LOW);
    digitalWrite(releB, ordreMotorB ? HIGH : LOW);
  } else {
    // Mode normal: Apliquem directament les ordres que arriben de l'emissor
    digitalWrite(releA, ordreMotorA ? HIGH : LOW);
    digitalWrite(releB, ordreMotorB ? HIGH : LOW);
  }

  delay(10);
}