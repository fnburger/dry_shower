#include <FastLED.h>

// --- Configuration ---
#define POT_PIN     32      
#define VIBE_PIN    2       
#define SPEAKER_PIN 25      
#define SWITCH_PIN  13      

// LED Strip Configuration (WS2801)
#define DATA_PIN    27      
#define CLOCK_PIN   14      
#define NUM_LEDS    41      
#define BRIGHTNESS  120     

CRGB leds[NUM_LEDS];

// --- Settings ---
const int RAIN_IRAN = 30;    
const int RAIN_AUSTRIA = 65; 
const int RAIN_JAPAN = 100; 

// --- Variables ---
float currentWaterLevel = 0; 
unsigned long lastSoundNote = 0;
int soundInterval = 100;
bool soundIsPlaying = false; 
unsigned long lastDebugPrint = 0; // Timer for Serial Debug

void setup() {
  Serial.begin(115200);
  
  FastLED.addLeds<WS2801, DATA_PIN, CLOCK_PIN, RGB>(leds, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);
  FastLED.clear();
  FastLED.show();

  pinMode(POT_PIN, INPUT);
  pinMode(VIBE_PIN, OUTPUT);
  pinMode(SWITCH_PIN, INPUT_PULLUP);

  Serial.println("--- DEBUG MODE: TELEPHONE SWITCH TEST ---");
}

void loop() {
  unsigned long currentMillis = millis();
  
  // Read Switch: LOW means Showering (Switch pressed/Head removed)
  int rawSwitchState = digitalRead(SWITCH_PIN);
  bool isShowering = (rawSwitchState == LOW);

  // ==========================================
  // DEBUG FEEDBACK
  // ==========================================
  if (currentMillis - lastDebugPrint > 100) {
    Serial.print("Switch Pin: ");
    Serial.print(rawSwitchState == LOW ? "LOW (Active)" : "HIGH (Idle)");
    Serial.print(" | Water: ");
    Serial.println(currentWaterLevel);
    lastDebugPrint = currentMillis;
  }

  // ==========================================
  // 1. WATER LEVEL & MOTOR LOGIC
  // ==========================================
  if (!isShowering) {
    long potSum = 0;
    for (int i = 0; i < 10; i++) potSum += analogRead(POT_PIN);
    int potValue = potSum / 10;

    int percent = 0;
    if (potValue < 1365) percent = RAIN_IRAN;
    else if (potValue < 2730) percent = RAIN_AUSTRIA;
    else percent = RAIN_JAPAN;

    currentWaterLevel = (percent * NUM_LEDS) / 100.0;
    analogWrite(VIBE_PIN, 0); 
    
  } else {
    if (currentWaterLevel > 0) {
      currentWaterLevel -= 0.015; 
      analogWrite(VIBE_PIN, 200); 
    } else {
      analogWrite(VIBE_PIN, 0); 
      currentWaterLevel = 0;
    }
  }

  // ==========================================
  // 2. SOUND EFFECTS
  // ==========================================
  if (currentMillis - lastSoundNote > soundInterval) {
    if (isShowering && currentWaterLevel > 0) {
      tone(SPEAKER_PIN, random(600, 1800)); 
      soundInterval = random(20, 40);
      soundIsPlaying = true;
    } else if (isShowering && currentWaterLevel <= 0) {
      tone(SPEAKER_PIN, 1200); 
      soundInterval = 500;
      soundIsPlaying = true;
    } else {
      tone(SPEAKER_PIN, random(150, 500)); 
      soundInterval = random(600, 1500);
      soundIsPlaying = true;
    }
    lastSoundNote = currentMillis;
  }

  if (soundIsPlaying && (currentMillis - lastSoundNote > 15)) {
    noTone(SPEAKER_PIN);
    soundIsPlaying = false;
  }

  // ==========================================
  // 3. LED STRIP DISPLAY
  // ==========================================
  FastLED.clear();
  int ledsToDisplay = (int)currentWaterLevel;
  for (int i = 0; i < ledsToDisplay; i++) {
    if (i < 3) leds[i] = CRGB::Red;
    else leds[i] = CRGB::Blue;
  }
  if (isShowering && ledsToDisplay == 0) {
    if ((currentMillis / 250) % 2 == 0) fill_solid(leds, 3, CRGB::Red);
  }
  FastLED.show();
}