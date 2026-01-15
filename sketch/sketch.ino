#include <FastLED.h>

// --- Configuration ---
#define POT_PIN     32      // Potentiometer
#define VIBE_PIN    2       // Vibration Motor
#define SPEAKER_PIN 25      // Speaker/Buzzer
#define SWITCH_PIN  13      // Toggle Switch (LOW = Showering)

// LED Strip Configuration (WS2801)
#define DATA_PIN    27      
#define CLOCK_PIN   14      
#define NUM_LEDS    41      
#define BRIGHTNESS  120     

CRGB leds[NUM_LEDS];

// --- Settings ---
const int RAIN_IRAN = 30;    
const int RAIN_AUSTRIA = 65; 
const int RAIN_JAPAN = 100; // Set to 100 to ensure all 41 LEDs light up

// --- Variables ---
float currentWaterLevel = 0; 
unsigned long lastPulseTime = 0;
bool motorState = false;
unsigned long lastSoundNote = 0;
int soundInterval = 100;
unsigned long lastSerialPrint = 0;

void setup() {
  Serial.begin(115200);
  
  FastLED.addLeds<WS2801, DATA_PIN, CLOCK_PIN, RGB>(leds, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);
  FastLED.clear();
  FastLED.show();

  pinMode(POT_PIN, INPUT);
  pinMode(VIBE_PIN, OUTPUT);
  pinMode(SWITCH_PIN, INPUT_PULLUP);

  Serial.println("--- DRY SHOWER: DEPLETION SYSTEM ACTIVE ---");
}

void loop() {
  unsigned long currentMillis = millis();
  
  // Read Switch: LOW means Showering (Switch pressed/Head removed)
  bool isShowering = (digitalRead(SWITCH_PIN) == LOW);

  // ==========================================
  // 1. WATER LEVEL LOGIC
  // ==========================================
  if (!isShowering) {
    // --- MODE: SELECTING COUNTRY ---
    // Average Potentiometer
    long potSum = 0;
    for (int i = 0; i < 10; i++) potSum += analogRead(POT_PIN);
    int potValue = potSum / 10;

    int percent = 0;
    if (potValue < 1365) percent = RAIN_IRAN;
    else if (potValue < 2730) percent = RAIN_AUSTRIA;
    else percent = RAIN_JAPAN;

    // Map percentage to number of LEDs
    currentWaterLevel = (percent * NUM_LEDS) / 100.0;
    
    // Safety: Turn off motor when not showering
    digitalWrite(VIBE_PIN, LOW);

  } else {
    // --- MODE: SHOWERING (DEPLETION) ---
    if (currentWaterLevel > 0) {
      // DRAIN SPEED: Increase this number to make the water run out faster
      // 0.02 means it takes a few seconds to drain the whole strip
      currentWaterLevel -= 0.015; 

      // Motor Pulsing
      if (currentMillis - lastPulseTime >= 200) {
        lastPulseTime = currentMillis;
        motorState = !motorState;
        digitalWrite(VIBE_PIN, motorState ? HIGH : LOW);
      }
    } else {
      // Tank is Empty
      digitalWrite(VIBE_PIN, LOW);
      currentWaterLevel = 0;
    }
  }

  // ==========================================
  // 2. SOUND EFFECTS
  // ==========================================
  if (currentMillis - lastSoundNote > soundInterval) {
    if (isShowering && currentWaterLevel > 0) {
      tone(SPEAKER_PIN, random(800, 1600), 20); // High rush
      soundInterval = random(20, 50);
    } else if (isShowering && currentWaterLevel <= 0) {
      tone(SPEAKER_PIN, 200, 100); // Low "Empty" hum
      soundInterval = 500;
    } else {
      tone(SPEAKER_PIN, random(100, 400), 40); // Ambient rain
      soundInterval = random(500, 1500);
    }
    lastSoundNote = currentMillis;
  }

  // ==========================================
  // 3. LED STRIP DISPLAY
  // ==========================================
  FastLED.clear();
  
  // We use (int) to truncate the float so LEDs turn off one by one
  int ledsToDisplay = (int)currentWaterLevel;

  for (int i = 0; i < ledsToDisplay; i++) {
    if (i < 3) {
      leds[i] = CRGB::Red;  // Bottom 3 Red
    } else {
      leds[i] = CRGB::Blue; // Rest Blue
    }
  }

  // Visual "Empty" Alert: Flash the Red LEDs if showering with no water
  if (isShowering && ledsToDisplay == 0) {
    if ((currentMillis / 250) % 2 == 0) {
       leds[0] = CRGB::Red; leds[1] = CRGB::Red; leds[2] = CRGB::Red;
    }
  }

  FastLED.show();

  // ==========================================
  // 4. SERIAL DEBUG
  // ==========================================
  if (currentMillis - lastSerialPrint > 500) {
    Serial.print("Mode: "); Serial.print(isShowering ? "SHOWERING " : "SELECTING ");
    Serial.print("| Water: "); Serial.print(currentWaterLevel);
    Serial.println();
    lastSerialPrint = currentMillis;
  }
}