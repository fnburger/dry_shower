#include <FastLED.h>

// --- Configuration ---
#define POT_PIN     34      // Potentiometer Pin
#define VIBE_PIN    2       // Vibration Motor Pin 
#define SPEAKER_PIN 25      // Speaker/Buzzer Pin 
#define SWITCH_PIN  13      // Toggle Switch (Moved from Pin 4)

// LED Strip Configuration (WS2801) 
#define DATA_PIN    27      // DI on strip
#define CLOCK_PIN   14      // CI on strip
#define NUM_LEDS    41      // Total amount of LEDs
#define BRIGHTNESS  120     // Global brightness (0-255)

CRGB leds[NUM_LEDS];

// --- Settings ---
const int PULSE_SPEED = 200; // Motor pulse speed 
const int RAIN_IRAN = 30;    // ~30% full 
const int RAIN_AUSTRIA = 65; // ~65% full 
const int RAIN_JAPAN = 95;   // ~95% full

// --- Variables ---
unsigned long lastPulseTime = 0;
bool motorState = false;
unsigned long lastSoundNote = 0;
int soundInterval = 100;
unsigned long lastSerialPrint = 0; 

void setup() {
  Serial.begin(115200);
  
  // Initialize WS2801 LED Strip 
  FastLED.addLeds<WS2801, DATA_PIN, CLOCK_PIN, RGB>(leds, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);
  FastLED.clear();
  FastLED.show();

  Serial.println("--- DRY SHOWER TEST MODE ---");
  
  pinMode(POT_PIN, INPUT);
  pinMode(VIBE_PIN, OUTPUT);
  pinMode(SWITCH_PIN, INPUT_PULLUP);
}

void loop() {
  unsigned long currentMillis = millis();

  // ==========================================
  // 1. POTENTIOMETER & COUNTRY LOGIC
  // ==========================================
  // Average 10 readings to smooth noise
  long potSum = 0;
  for (int i = 0; i < 10; i++) {
    potSum += analogRead(POT_PIN);
    delay(1); 
  }
  int potValue = potSum / 10;

  int waterLevelPercent = 0;
  String country = "";
  
  // Determine Country based on Potentiometer
  if (potValue < 1365) {
    country = "Iran    "; 
    waterLevelPercent = RAIN_IRAN;
  } else if (potValue < 2730) {
    country = "Austria "; 
    waterLevelPercent = RAIN_AUSTRIA;
  } else {
    country = "Japan   "; 
    waterLevelPercent = RAIN_JAPAN;
  }


// ==========================================
  // 2. LED STRIP FEEDBACK
  // ==========================================
  // Calculate how many LEDs to light
  int ledsToLight = (waterLevelPercent * NUM_LEDS) / 100;
  
  // FIX: Force the tank to appear 100% full if the highest country is selected
  if (waterLevelPercent >= 95) {
    ledsToLight = NUM_LEDS;
  }

  FastLED.clear();
  for (int i = 0; i < ledsToLight; i++) {
    if (i < 3) {
      leds[i] = CRGB::Red;  // Bottom 3 LEDs suggest a low water alert [cite: 23, 25]
    } else {
      leds[i] = CRGB::Blue; // Blue LEDs suggest remaining water [cite: 23, 25]
    }
  }
  FastLED.show();

  // ==========================================
  // 3. SWITCH & MOTOR LOGIC
  // ==========================================
  bool switchUp = (digitalRead(SWITCH_PIN) == LOW); 

  if (switchUp) {
    // SHOWER ON: Pulsing motor
    if (currentMillis - lastPulseTime >= PULSE_SPEED) {
      lastPulseTime = currentMillis;
      motorState = !motorState;
      digitalWrite(VIBE_PIN, motorState ? HIGH : LOW);
    }
  } else {
    // SHOWER OFF: Force motor OFF
    digitalWrite(VIBE_PIN, LOW);
    motorState = false; 
  }

  // ==========================================
  // 4. SOUND LOGIC
  // ==========================================
  if (currentMillis - lastSoundNote > soundInterval) {
    bool isShowering = switchUp;
    int freqMin, freqMax, duration, intervalMin, intervalMax;
    
    if (!isShowering) {
      // Rain sounds
      freqMin = 100; freqMax = 600; duration = 50; intervalMin = 50; intervalMax = 150;
    } else {
      // Shower sounds
      freqMin = 800; freqMax = 2000; duration = 30; intervalMin = 20; intervalMax = 60;
    }
    
    tone(SPEAKER_PIN, random(freqMin, freqMax + 1), duration);
    lastSoundNote = currentMillis;
    soundInterval = random(intervalMin, intervalMax + 1);
  }

  // ==========================================
  // 5. SERIAL MONITOR (200ms)
  // ==========================================
  if (currentMillis - lastSerialPrint > 200) {
    String switchState = switchUp ? "ON (Shower)" : "OFF (Rain)";
    Serial.print("Pot: "); Serial.print(potValue);
    Serial.print(" | "); Serial.print(country);
    Serial.print(" | Water: "); Serial.print(waterLevelPercent);
    Serial.print("% | LEDs: "); Serial.print(ledsToLight);
    Serial.print(" | Switch: "); Serial.println(switchState);
    lastSerialPrint = currentMillis;
  }
}