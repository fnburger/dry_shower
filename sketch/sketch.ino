#include <FastLED.h>

// --- Configuration ---
#define POT_PIN     32      
#define VIBE_PIN    2       
#define SPEAKER_PIN 25      
#define SWITCH_PIN  13      

#define DATA_PIN    27      
#define CLOCK_PIN   14      
#define NUM_LEDS    41      
#define BRIGHTNESS  120     

CRGB leds[NUM_LEDS];

// --- Settings ---
const int RAIN_IRAN = 30;    
const int RAIN_AUSTRIA = 65; 
const int RAIN_TAIWAN = 100; 
const float DRAIN_SPEED = 0.01;

// --- Variables ---
float currentWaterLevel = 0; 
int lastTargetLevel = 0;      // Track pot changes
unsigned long lastSoundNote = 0;
int soundInterval = 100;
bool soundIsPlaying = false; 

// Drip Animation Variables
float dripPosition = -1;      
unsigned long lastDripMove = 0; 
const int DRIP_SPEED = 30;     

void setup() {
  Serial.begin(115200);
  FastLED.addLeds<WS2801, DATA_PIN, CLOCK_PIN, RGB>(leds, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);
  FastLED.clear();
  FastLED.show();

  pinMode(POT_PIN, INPUT);
  pinMode(VIBE_PIN, OUTPUT);
  pinMode(SWITCH_PIN, INPUT_PULLUP);
}

void loop() {
  unsigned long currentMillis = millis();
  bool isShowering = (digitalRead(SWITCH_PIN) == LOW);

  // 1. Calculate Target from Potentiometer
  long potSum = 0;
  for (int i = 0; i < 10; i++) potSum += analogRead(POT_PIN);
  int potValue = potSum / 10;

  int percent = 0;
  if (potValue < 1365) percent = RAIN_IRAN;
  else if (potValue < 2730) percent = RAIN_AUSTRIA;
  else percent = RAIN_TAIWAN;

  float targetLevel = (percent * NUM_LEDS) / 100.0;

  // ==========================================
  // 1. WATER LEVEL & MOTOR LOGIC
  // ==========================================
  if (isShowering) {
    // --- MODE: DEPLETING ---
    dripPosition = -1; 
    if (currentWaterLevel > 0) {
      currentWaterLevel -= DRAIN_SPEED; // Drain water
      analogWrite(VIBE_PIN, 200); // Buzzing motor
    } else {
      analogWrite(VIBE_PIN, 0); 
      currentWaterLevel = 0;
    }
  } 
  else {
    // --- MODE: IDLE / REFILL ---
    analogWrite(VIBE_PIN, 0); 

    // BUG FIX: If the user turns the POT, jump instantly to that level
    if ((int)targetLevel != lastTargetLevel) {
      currentWaterLevel = targetLevel; 
      lastTargetLevel = (int)targetLevel;
      dripPosition = -1; // Stop drip if we manually changed countries
    } 
    // Otherwise, if we just finished a shower, refill slowly
    else if (currentWaterLevel < targetLevel) {
      currentWaterLevel += 0.005; // Refill rate
      
      // Start drip animation during slow refill
      if (dripPosition < 0) dripPosition = NUM_LEDS - 1;
    } 
    else {
      dripPosition = -1; // Fully charged
    }
  }

  // ==========================================
  // 2. DRIP MOVEMENT
  // ==========================================
  if (dripPosition >= 0 && currentMillis - lastDripMove > DRIP_SPEED) {
    lastDripMove = currentMillis;
    dripPosition -= 1.0; 
    if (dripPosition <= currentWaterLevel) {
      // SYNCED SOUND: Play tinkle only when LED hits the water surface
      tone(SPEAKER_PIN, random(1200, 1800), 15);
      soundIsPlaying = true;
      lastSoundNote = currentMillis;

      dripPosition = (currentWaterLevel < targetLevel) ? NUM_LEDS - 1 : -1;
    }
  }

  // ==========================================
  // 3. SOUND EFFECTS
  // ==========================================
  if (currentMillis - lastSoundNote > soundInterval) {
    if (isShowering && currentWaterLevel > 0) {
      tone(SPEAKER_PIN, random(600, 1800)); 
      soundInterval = random(20, 40);
      soundIsPlaying = true;
      lastSoundNote = currentMillis;
    } else if (isShowering && currentWaterLevel <= 0) {
      tone(SPEAKER_PIN, 1200); // Tank empty beep
      soundInterval = 500;
      soundIsPlaying = true;
      lastSoundNote = currentMillis;
    } else if (!isShowering && dripPosition < 0) {
      // Ambient rain only when NOT refilling (refill sound handled by drip move)
      tone(SPEAKER_PIN, random(150, 500)); 
      soundInterval = random(800, 2000);
      soundIsPlaying = true;
      lastSoundNote = currentMillis;
    }
  }

  if (soundIsPlaying && (currentMillis - lastSoundNote > 15)) {
    noTone(SPEAKER_PIN);
    soundIsPlaying = false;
  }

  // ==========================================
  // 4. LED STRIP DISPLAY
  // ==========================================
  FastLED.clear();
  int ledsToDisplay = (int)currentWaterLevel;
  for (int i = 0; i < ledsToDisplay; i++) {
    if (i < 3) leds[i] = CRGB::Red;
    else leds[i] = CRGB::Blue;
  }
  if (dripPosition >= 0) leds[(int)dripPosition] = CRGB::Cyan;
  if (isShowering && ledsToDisplay == 0) {
    if ((currentMillis / 250) % 2 == 0) fill_solid(leds, 3, CRGB::Red);
  }
  FastLED.show();
}