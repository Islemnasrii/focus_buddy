#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <FluxGarage_RoboEyes.h>
#include <cstring>

const int BUTTON = 27, LED = 25, BUZZER = 26;

class BufferedOLED : public Adafruit_SH1106G {
public:
  BufferedOLED() : Adafruit_SH1106G(128, 64, &Wire, -1) {}
  void display() override {}                 
  void flush() { Adafruit_SH1106G::display(); } 
};

BufferedOLED oled;
RoboEyes<BufferedOLED> eyes(oled);

enum Phase { FOCUS, SHORT, LONG };
Phase phase = FOCUS;
uint8_t sessions = 0;
uint32_t remaining = 1500000, lastTick = 0, transition = 0;
bool running = false, screenOK = false;

uint32_t duration() {
  return phase == FOCUS ? 1500000UL : phase == SHORT ? 300000UL : 900000UL;
}

void advance(uint32_t now) {
  if (phase == FOCUS) { sessions++; phase = (sessions % 4 == 0) ? LONG : SHORT; }
  else phase = FOCUS;
  remaining = duration();
  transition = now;
  lastTick = now;
}

void draw(uint32_t now) {
  static int oldPhase = -1;
  static bool previousPulse = false;
  static uint8_t previousFrame[1024];
  static bool first = true;


if (oldPhase != phase) {
  oldPhase = phase;
  
  if (phase == FOCUS) {
    eyes.setMood(HAPPY);
    eyes.setHeight(16, 16);
    eyes.setAutoblinker(true, 3, 3);
    eyes.setIdleMode(false, 0, 0);
    eyes.setHFlicker(true, 0);      
    eyes.setVFlicker(false, 0);
    eyes.setCuriosity(true);
    eyes.setPosition(DEFAULT);
    eyes.blink();                    
  }
  else if (phase == SHORT) {
    eyes.setMood(DEFAULT);
    eyes.setHeight(24, 24);
    eyes.setAutoblinker(true, 5, 3);
    eyes.setIdleMode(true, 1, 2);
    eyes.setHFlicker(false, 0);
    eyes.setVFlicker(false, 0);
    eyes.setCuriosity(true);         
    eyes.setPosition(DEFAULT);
    eyes.anim_laugh();               
  }
  else { 
    eyes.setMood(TIRED);
    eyes.setHeight(18, 18);
    eyes.setAutoblinker(true, 6, 3);
    eyes.setIdleMode(false, 0, 0);
    eyes.setHFlicker(false, 0);
    eyes.setVFlicker(true, 1);       
    eyes.setCuriosity(false);
    eyes.setPosition(DEFAULT);
  }
}

  uint32_t age = now - transition;
  bool pulse = transition != 0 && (age < 120 || (age >= 260 && age < 380) || (age >= 520 && age < 820));
  if (pulse && !previousPulse) eyes.blink();
  previousPulse = pulse;

  eyes.update();

  oled.fillRect(0, 0, 128, 9, SH110X_BLACK);
  oled.fillRect(0, 48, 128, 16, SH110X_BLACK);


  oled.setTextSize(1);
  oled.setTextColor(SH110X_WHITE);
  oled.setCursor(0, 0);
  oled.print(phase == FOCUS ? "FOCUS" : phase == SHORT ? "BREAK" : "REST");

  uint32_t sec = (remaining + 999) / 1000;
  char text[12];
  snprintf(text, sizeof(text), "%02lu:%02lu",
           (unsigned long)(sec / 60), (unsigned long)(sec % 60));
  oled.setCursor(96, 0);
  oled.print(text);

  int progress = (uint64_t)(duration() - remaining) * 116 / duration();
  oled.drawRect(5, 49, 118, 5, SH110X_WHITE);
  if (progress) oled.fillRect(6, 50, progress, 3, SH110X_WHITE);

  oled.setCursor(0, 57);
  oled.print(running ? "HOLD: SKIP" : "TAP: START/PAUSE");

  uint8_t *buffer = oled.getBuffer();
  if (first || memcmp(previousFrame, buffer, 1024) != 0) {
    memcpy(previousFrame, buffer, 1024);
    oled.flush();
    first = false;
  }
}

void setup() {
  pinMode(BUTTON, INPUT_PULLUP);
  pinMode(LED, OUTPUT);
  pinMode(BUZZER, OUTPUT);

  Wire.begin(21, 22);
  screenOK = oled.begin(0x3C, true);
  if (!screenOK) screenOK = oled.begin(0x3D, true);

  if (screenOK) {
    eyes.begin(128, 48, 30);       
    eyes.setWidth(28, 28);
    eyes.setSpacebetween(26);
    eyes.setBorderradius(5, 5);
    eyes.open();
  }

  randomSeed(micros());
  lastTick = millis();
}

void loop() {
  uint32_t now = millis();
  uint32_t elapsed = now - lastTick;
  lastTick = now;

  if (running) {
    if (elapsed >= remaining) advance(now);
    else remaining -= elapsed;
  }

  static bool rawOld = false, stable = false, held = false;
  static uint32_t changed = 0, pressed = 0;
  bool raw = digitalRead(BUTTON) == LOW;

  if (raw != rawOld) { rawOld = raw; changed = now; }

  if (now - changed >= 30 && stable != raw) {
    stable = raw;
    if (stable) {
      pressed = now;
      held = false;
    } else if (!held) {
      running = !running;
    }
  }

  if (stable && !held && now - pressed >= 1000) {
    held = true;
    advance(now);
  }

  uint32_t age = now - transition;
  bool alert = transition != 0 && age < 1000;
  bool pulse = alert && ((age < 120) || (age >= 260 && age < 380) || (age >= 520 && age < 820));

  static bool oldPulse = false;
  if (pulse != oldPulse) {
    oldPulse = pulse;
    if (pulse) tone(BUZZER, phase == FOCUS ? 1800 : 1200);
    else noTone(BUZZER);
  }

  digitalWrite(LED, alert ? pulse : running && phase == FOCUS);

  if (screenOK) draw(now);
  delay(10);
}