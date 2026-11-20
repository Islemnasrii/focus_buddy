#focus_buddy
A Pomodoro timer that **looks back at you**.
focus_buddy turns a 128×64 OLED into a pair of expressive robot eyes that change
with every phase of the Pomodoro technique. Built on an ESP32 with the FluxGarage RoboEyes library, it doubles as a
minimal desk companion.
## DEMO
https://drive.google.com/file/d/1hphBUBG85Bz2jmnikuV8uQpTLheFCjiw/view?usp=drive_link
## Features

- **25 / 5 / 15 Pomodoro cycle** with automatic long break every 4 focus sessions
- **Three distinct eye expressions** driven by phase:
  - **FOCUS** — narrow, locked-in, slightly tense eyes
  - **SHORT BREAK** — relaxed, wandering, curious eyes
  - **LONG BREAK** — sleepy, half-lidded, slow-blinking eyes
- **Single-button control** — short tap = start/pause, long hold = skip phase
- **Buzzer + LED feedback** on every phase transition

## Hardware

| Component | Details |
|---|---|
| Microcontroller | ESP32  |
| Display | SH1106 128×64 OLED |
| Button | push button  |
| LED | + 220–330 Ω resistor |
| Buzzer | KY-006 **passive** buzzer module |

### Wiring

| Component | Pin | ESP32 GPIO |
|---|---|---|
| OLED | SDA | 21 |
| OLED | SCL | 22 |
| OLED | VCC / GND | 3.3V / GND |
| Button | one side | 27 |
| Button | other side | GND |
| LED | anode (via resistor) | 25 |
| LED | cathode | GND |
| Buzzer | S (signal) | 26 |
| Buzzer | GND | GND |
