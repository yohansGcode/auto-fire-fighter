/*
 * ============================================================
 *  AUTO FIRE FIGHTER
 *  Arduino Uno (ATmega328P)
 * ------------------------------------------------------------
 *  Monitors air quality using an MQ-2 smoke/gas sensor.
 *  When gas or smoke is detected above a safe threshold,
 *  it immediately activates a water pump via a relay module,
 *  sounds a buzzer alarm, and lights up a red LED.
 *  When the air clears, everything shuts off automatically.
 *
 *  HOW IT WORKS:
 *    1. MQ-2 sensor continuously reads smoke/gas levels
 *    2. If reading exceeds DANGER_THRESHOLD → ALARM ON
 *    3. Red LED lights up
 *    4. Buzzer sounds continuously
 *    5. Relay triggers → water pump activates → water sprays
 *    6. When reading drops below SAFE_THRESHOLD → ALL OFF
 *    7. Green LED lights up → system back to standby
 *
 *  WIRING SUMMARY:
 *    MQ-2 Analog out (A0)   → A0
 *    MQ-2 VCC               → 5V
 *    MQ-2 GND               → GND
 *    Relay IN               → D7
 *    Relay VCC              → 5V
 *    Relay GND              → GND
 *    Water Pump (+)         → Relay COM
 *    Water Pump (-)         → Battery GND
 *    Red LED (+)            → D6 (via 220Ω resistor)
 *    Green LED (+)          → D5 (via 220Ω resistor)
 *    LED (-)                → GND
 *    Buzzer (+)             → D8
 *    Buzzer (-)             → GND
 *
 *  IMPORTANT — RELAY WIRING FOR PUMP:
 *    The pump runs on its own battery (do NOT power from Arduino)
 *    Relay COM  → Pump positive wire
 *    Relay NO   → Pump battery positive
 *    Relay GND  → Shared GND with Arduino
 *    When relay triggers, it completes the pump circuit.
 * ============================================================
 */

#include <Arduino.h>

// ── Pin Definitions ─────────────────────────────────────────
const int MQ2_PIN      = A0;   // MQ-2 analog output
const int RELAY_PIN    = 7;    // Relay IN — controls water pump
const int LED_RED_PIN  = 6;    // Red LED — danger indicator
const int LED_GRN_PIN  = 5;    // Green LED — standby indicator
const int BUZZER_PIN   = 8;    // Buzzer — alarm

// ── Detection Thresholds ─────────────────────────────────────
// MQ-2 reads 0–1023. Higher = more smoke/gas in the air.
// DANGER_THRESHOLD: above this → activate fire response
// SAFE_THRESHOLD:   below this → deactivate (must be lower
//                  than danger to prevent rapid on/off cycling)
//
// These defaults work for most MQ-2 sensors in open air.
// If false alarms occur, increase DANGER_THRESHOLD.
// If detection is too slow, decrease DANGER_THRESHOLD.
const int DANGER_THRESHOLD = 400;   // trigger alarm + pump
const int SAFE_THRESHOLD   = 300;   // clear alarm + pump off

// ── Timing ───────────────────────────────────────────────────
const int SAMPLE_INTERVAL  = 500;   // ms between readings
const int BUZZER_BEEP_ON   = 200;   // ms buzzer on during alarm
const int BUZZER_BEEP_OFF  = 100;   // ms buzzer off during alarm

// ── State ────────────────────────────────────────────────────
bool alarmActive       = false;
unsigned long lastRead = 0;
unsigned long lastBeep = 0;
bool buzzerState       = false;

// ============================================================
//  SETUP
// ============================================================
void setup() {
  Serial.begin(9600);

  pinMode(RELAY_PIN,   OUTPUT);
  pinMode(LED_RED_PIN, OUTPUT);
  pinMode(LED_GRN_PIN, OUTPUT);
  pinMode(BUZZER_PIN,  OUTPUT);

  // Safe starting state — pump OFF, alarm OFF
  digitalWrite(RELAY_PIN,   HIGH);  // HIGH = relay OFF (most relay modules are active LOW)
  digitalWrite(LED_RED_PIN, LOW);
  digitalWrite(BUZZER_PIN,  LOW);

  // MQ-2 needs ~2 minutes to warm up its heating element
  // for accurate readings on first power-on
  Serial.println("============================================");
  Serial.println("  AUTO FIRE FIGHTER");
  Serial.println("  Warming up MQ-2 sensor...");
  Serial.println("============================================");

  // Flash green LED during warm-up to show system is alive
  for (int i = 0; i < 20; i++) {
    digitalWrite(LED_GRN_PIN, HIGH); delay(2000);
    digitalWrite(LED_GRN_PIN, LOW);  delay(500);
  }
  // After warm-up, keep green LED solid = standby ready
  digitalWrite(LED_GRN_PIN, HIGH);

  Serial.println("  Ready! Monitoring for smoke/gas...");
  Serial.println("  Reading, Threshold, Status");
}

// ============================================================
//  MAIN LOOP
// ============================================================
void loop() {
  unsigned long now = millis();

  // Take a reading every SAMPLE_INTERVAL milliseconds
  if (now - lastRead >= SAMPLE_INTERVAL) {
    lastRead = now;

    int gasLevel = analogRead(MQ2_PIN);

    Serial.print("Gas level: ");
    Serial.print(gasLevel);
    Serial.print(" / Threshold: ");
    Serial.print(DANGER_THRESHOLD);
    Serial.print(" / Status: ");

    // ── DANGER detected ────────────────────────────────────
    if (gasLevel >= DANGER_THRESHOLD && !alarmActive) {
      alarmActive = true;
      activateFireResponse();
      Serial.println("!!! DANGER — FIRE RESPONSE ACTIVE !!!");
    }

    // ── AIR CLEARED ────────────────────────────────────────
    else if (gasLevel < SAFE_THRESHOLD && alarmActive) {
      alarmActive = false;
      deactivateFireResponse();
      Serial.println("CLEAR — System back to standby.");
    }

    // ── NORMAL ─────────────────────────────────────────────
    else if (!alarmActive) {
      Serial.println("Normal.");
    }
  }

  // Non-blocking buzzer beeping during alarm
  if (alarmActive) {
    pulseBuzzer(now);
  }
}

// ============================================================
//  ACTIVATE FIRE RESPONSE
//  Turns on pump, red LED, begins buzzer alarm
// ============================================================
void activateFireResponse() {
  digitalWrite(RELAY_PIN,   LOW);   // LOW = relay ON → pump starts
  digitalWrite(LED_RED_PIN, HIGH);  // Red LED on
  digitalWrite(LED_GRN_PIN, LOW);   // Green LED off
  Serial.println("  → Pump ON | Red LED ON | Buzzer alarming");
}

// ============================================================
//  DEACTIVATE FIRE RESPONSE
//  Turns off pump, red LED, buzzer — back to standby
// ============================================================
void deactivateFireResponse() {
  digitalWrite(RELAY_PIN,   HIGH);  // HIGH = relay OFF → pump stops
  digitalWrite(LED_RED_PIN, LOW);   // Red LED off
  digitalWrite(LED_GRN_PIN, HIGH);  // Green LED on = standby
  digitalWrite(BUZZER_PIN,  LOW);   // Buzzer off
  buzzerState = false;
  Serial.println("  → Pump OFF | Green LED ON | Buzzer off");
}

// ============================================================
//  NON-BLOCKING BUZZER PULSE
//  Beeps repeatedly during alarm without using delay()
//  This keeps the sensor reading loop running while beeping
// ============================================================
void pulseBuzzer(unsigned long now) {
  if (buzzerState) {
    // Buzzer is ON — check if it's time to turn it OFF
    if (now - lastBeep >= BUZZER_BEEP_ON) {
      digitalWrite(BUZZER_PIN, LOW);
      buzzerState = false;
      lastBeep = now;
    }
  } else {
    // Buzzer is OFF — check if it's time to turn it ON
    if (now - lastBeep >= BUZZER_BEEP_OFF) {
      digitalWrite(BUZZER_PIN, HIGH);
      buzzerState = true;
      lastBeep = now;
    }
  }
}
