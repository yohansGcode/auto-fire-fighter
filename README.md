# 🔥 Auto Fire Fighter

> An Arduino-powered fire and gas detection system that automatically activates a water pump the moment smoke or dangerous gas is detected — with a red LED alarm and buzzer for immediate alert.

![Arduino](https://img.shields.io/badge/Platform-Arduino-00979D?style=flat&logo=arduino&logoColor=white)
![Sensor](https://img.shields.io/badge/Sensor-MQ--2%20Smoke%2FGas-FF4444?style=flat)
![Status](https://img.shields.io/badge/Status-Completed-brightgreen?style=flat)

---

## 📌 Overview

House fires and gas leaks claim lives because detection and response are too slow. The **Auto Fire Fighter** addresses this by using an MQ-2 smoke and gas sensor connected to an Arduino ATmega328P to monitor air quality continuously. The moment gas or smoke exceeds a safe level, the system responds instantly — no human action required.

The response is threefold:
- 🔴 A red LED lights up as a visual danger indicator
- 🔔 A buzzer sounds a continuous alarm
- 💧 A relay triggers a water pump, spraying water immediately

When the air clears, everything shuts off automatically and the system returns to green standby mode.

---

## ✨ Features

- 🌫️ Continuous real-time smoke and gas monitoring via MQ-2 sensor
- ⚡ Instant automated response — zero human intervention needed
- 💧 Water pump activated via relay module for fire suppression
- 🔴 Red LED danger indicator and green standby indicator
- 🔔 Non-stop buzzer alarm during active threat
- ✅ Auto-reset when air returns to safe levels
- 📺 Serial Monitor output for live readings and status

---

## 🧰 Components Used

| Component | Quantity | Purpose |
|---|---|---|
| Arduino Uno (ATmega328P) | 1 | Main microcontroller |
| MQ-2 Smoke/Gas Sensor | 1 | Detects smoke, LPG, CO, and other gases |
| 5V Relay Module | 1 | Switches water pump on/off safely |
| Small DC Water Pump | 1 | Sprays water when triggered |
| Red LED | 1 | Danger/alarm indicator |
| Green LED | 1 | Standby/safe indicator |
| Passive Buzzer | 1 | Audible alarm |
| 220Ω Resistors | 2 | Current limiting for LEDs |
| 9V Battery (x2) | 2 | One for Arduino, one for pump |
| Jumper Wires | — | Connections |
| Breadboard | 1 | Prototyping |
| Small container + tubing | 1 | Water reservoir and spray nozzle |

> **Note:** The water pump must run on its own separate battery — never power it directly from the Arduino 5V pin, as it draws too much current and will damage the board. The relay safely bridges the two circuits.

---

## 🔌 Circuit Diagram

```
                        +5V   GND
                         |     |
              ┌──────────┴─────┴──────────┐
              │       ARDUINO UNO         │
              │                           │
              │  A0 ──────────────────────┼──► MQ-2 Analog Out (A0)
              │                           │
              │  D7 ───────────────────── ┼──► Relay IN
              │  D6 ───────────────────── ┼──► Red LED (+) → 220Ω → GND
              │  D5 ───────────────────── ┼──► Green LED (+) → 220Ω → GND
              │  D8 ───────────────────── ┼──► Buzzer (+)
              │                           │
              │  5V ───────────────────── ┼──► MQ-2 VCC
              │  5V ───────────────────── ┼──► Relay VCC
              │  GND ──────────────────── ┼──► MQ-2 GND
              │  GND ──────────────────── ┼──► Relay GND
              │  GND ──────────────────── ┼──► Buzzer (-)
              │  GND ──────────────────── ┼──► LED (-)
              │                           │
              │  VIN ──────────────────── ┼──► 9V Battery 1 (+)
              └───────────────────────────┘

  MQ-2 Sensor — 4 pins:
  ┌──────────────────────────┐
  │ VCC  → Arduino 5V        │
  │ GND  → Arduino GND       │
  │ A0   → Arduino A0        │  ← use the analog pin, not D0
  │ D0   → not connected     │
  └──────────────────────────┘

  5V Relay Module:
  ┌───────────────────────────────────┐
  │ IN   → Arduino D7                 │
  │ VCC  → Arduino 5V                 │
  │ GND  → Arduino GND                │
  │ COM  → Water pump (+) wire        │
  │ NO   → Battery 2 positive (+)     │  ← pump's own battery
  └───────────────────────────────────┘
  (Battery 2 negative → pump (-) wire → shared GND)
```

---

## ⚙️ How It Works

### 1. MQ-2 Warm-Up
On first power-on, the MQ-2 sensor's internal heating element needs about 2 minutes to reach operating temperature for accurate readings. During this time the green LED flashes slowly to show the system is alive. Once warm, the green LED stays solid — system is in standby.

### 2. Continuous Monitoring
Every 500ms the Arduino reads the analog voltage from the MQ-2 sensor (range 0–1023). Higher values mean more smoke or gas in the air. Readings are printed to the Serial Monitor in real time.

### 3. Danger Detected
When the reading exceeds `DANGER_THRESHOLD` (default: 400):
- The relay pin goes LOW → relay activates → pump circuit closes → **water sprays**
- Red LED turns ON
- Green LED turns OFF
- Buzzer begins pulsing in a repeating alarm pattern

### 4. Continuous Response
While the reading stays above `SAFE_THRESHOLD` (default: 300), all systems remain active. The two separate thresholds prevent the pump from rapidly switching on and off at the boundary.

### 5. Air Cleared — Auto Reset
When the reading drops below `SAFE_THRESHOLD`:
- Relay goes HIGH → pump switches OFF
- Red LED turns OFF
- Green LED turns back ON
- Buzzer stops
- System returns to standby monitoring immediately

```
  RESPONSE FLOW

  [Power ON] → [MQ-2 Warm-up ~2min] → [Green LED solid: Standby]
       ↓
  [Read MQ-2 every 500ms]
       ↓
  [Reading ≥ 400?]
       ↓ YES
  [Red LED ON] + [Buzzer alarm] + [Relay → Pump ON → Water sprays]
       ↓
  [Keep monitoring...]
       ↓
  [Reading < 300?]
       ↓ YES
  [All OFF] → [Green LED ON] → [Back to standby]
```

---

## 💻 Code

The main sketch is in [`auto_fire_fighter.ino`](./auto_fire_fighter.ino).

### No Extra Libraries Needed

This project uses only built-in Arduino functions — no library installation required:

```cpp
analogRead()       // reads MQ-2 sensor value
digitalWrite()     // controls relay, LEDs, buzzer
millis()           // non-blocking timing for buzzer pulse
```

### Adjustable Thresholds

The two key settings at the top of the sketch are easy to tune:

```cpp
const int DANGER_THRESHOLD = 400;  // above this → fire response ON
const int SAFE_THRESHOLD   = 300;  // below this → fire response OFF
```

If you get false alarms in normal air → **increase** `DANGER_THRESHOLD`.
If detection feels too slow → **decrease** `DANGER_THRESHOLD`.

### Core Detection Logic

```cpp
void loop() {
  int gasLevel = analogRead(MQ2_PIN);

  // Danger: activate pump, LED, buzzer
  if (gasLevel >= DANGER_THRESHOLD && !alarmActive) {
    alarmActive = true;
    activateFireResponse();
  }

  // Air cleared: shut everything off
  else if (gasLevel < SAFE_THRESHOLD && alarmActive) {
    alarmActive = false;
    deactivateFireResponse();
  }

  // Keep buzzer pulsing without blocking the sensor loop
  if (alarmActive) {
    pulseBuzzer(millis());
  }
}
```

### Why `millis()` Instead of `delay()`

The buzzer alarm uses `millis()` for timing instead of `delay()`. This is important — `delay()` would pause the entire program, meaning the sensor would stop reading while the buzzer is on. Using `millis()` keeps the sensor loop running continuously even while the buzzer is beeping.

---

## 📊 Serial Monitor Output

Connect to a computer and open Serial Monitor at **9600 baud** to see live readings:

```
============================================
  AUTO FIRE FIGHTER
  Warming up MQ-2 sensor...
============================================
  Ready! Monitoring for smoke/gas...
  Reading, Threshold, Status
Gas level: 187 / Threshold: 400 / Status: Normal.
Gas level: 193 / Threshold: 400 / Status: Normal.
Gas level: 421 / Threshold: 400 / Status: !!! DANGER — FIRE RESPONSE ACTIVE !!!
  → Pump ON | Red LED ON | Buzzer alarming
Gas level: 468 / Threshold: 400 / Status: !!! DANGER — FIRE RESPONSE ACTIVE !!!
Gas level: 389 / Threshold: 400 / Status: !!! DANGER — FIRE RESPONSE ACTIVE !!!
Gas level: 267 / Threshold: 400 / Status: CLEAR — System back to standby.
  → Pump OFF | Green LED ON | Buzzer off
Gas level: 201 / Threshold: 400 / Status: Normal.
```

---

## 🔮 Future Improvements

- [ ] Add a temperature sensor (DHT11) for combined heat and smoke detection
- [ ] Add an SMS/Wi-Fi alert via ESP8266 to notify the homeowner remotely
- [ ] Add a flame sensor (IR-based) for direct fire detection
- [ ] Increase water reservoir size for longer spray duration
- [ ] Add a manual override button to stop the pump if needed
- [ ] Mount in a weatherproof enclosure for outdoor use

---

## ⚠️ Safety Notes

- This is a **prototype and demonstration project** — not a certified fire suppression system
- Always follow proper electrical safety when working with relays and pumps
- Keep the water reservoir and electronics separated to prevent short circuits
- Never use this as a substitute for a professional fire alarm or sprinkler system
- Test the system outdoors or in a well-ventilated area when calibrating

---

## 🌍 Real-World Impact

Automated fire detection and suppression systems save thousands of lives every year — but commercial systems are expensive and inaccessible in many communities. This project demonstrates that the core principles of fire detection and automated response can be implemented with affordable hardware, opening up the possibility of low-cost early-warning systems for homes, small businesses, and schools in underserved areas.

---

## 👨‍💻 Author

Yohans Seife
- GitHub: yohansGcode(https://github.com/yohansGcode)
- Built for: Kallamino Special Highschool science fair competition

---

*Built with ❤️ and Arduino — because every second counts in an emergency.*
