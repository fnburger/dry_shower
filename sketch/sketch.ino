#include <FastLED.h>
#include <DFRobotDFPlayerMini.h>
#include <HardwareSerial.h>

// --- Configuration ---
#define VIBE_PIN 2
#define SHOWER_LED_PIN 23
#define SWITCH_PIN_SHOWER 13
#define SWITCH_PIN_COUNTRY 12

// LED strip
#define DATA_PIN 27
#define CLOCK_PIN 14
#define NUM_LEDS 41
#define BRIGHTNESS 120

// DFPlayer uses UART2: RX (GPIO 16), TX (GPIO 17) is standard for ESP32 UART2
#define RX_PIN 16
#define TX_PIN 17

HardwareSerial dfSerial(2);
DFRobotDFPlayerMini df;
CRGB leds[NUM_LEDS];

// --- Settings ---
const int RAIN_IRAN = 30;
const int RAIN_AUSTRIA = 65;
const int RAIN_INDONASIA = 100;
const float DRAIN_SPEED = 0.005;
const float TRANSITION_SPEED = 0.1; 
const float DRAIN_SPEED = 0.005;   // Slow "Rain" refill speed
const float TRANSITION_SPEED = 0.2; // Fast "Button" animation speed

// --- Variables ---
float currentWaterLevel = 0;
int lastTargetLevel = 0;
float dripPosition = -1;
unsigned long lastDripMove = 0;
const int DRIP_SPEED = 30;

bool isShowering = false;
bool lastSwitchShower = HIGH;
int countryIndex = 0;  // 0: IRAN, 1: AUSTRIA, 2: INDONASIA
bool lastSwitchCountry = HIGH;

// Idle Animation Variables
bool hasInteracted = false; 
float idleDripPos = NUM_LEDS - 1;

// Audio State Tracking
enum SoundState { SILENT, RAIN, SHOWER, EMPTY };
SoundState currentSound = SILENT;
bool isRefilling = false;

void setup() {
  Serial.begin(115200);
  FastLED.addLeds<WS2801, DATA_PIN, CLOCK_PIN, RGB>(leds, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);
  FastLED.clear();
  FastLED.show();

  // DFPlayer Init
  delay(5000);
  dfSerial.begin(9600, SERIAL_8N1, RX_PIN, TX_PIN);
  
  df.begin(dfSerial); 
  delay(200);
  df.reset();
  delay(1000); 

  df.volume(30);

  pinMode(VIBE_PIN, OUTPUT);
  pinMode(SHOWER_LED_PIN, OUTPUT);
  pinMode(SWITCH_PIN_SHOWER, INPUT_PULLUP);
  pinMode(SWITCH_PIN_COUNTRY, INPUT_PULLUP);
}

void loop() {
  unsigned long currentMillis = millis();

  // ------------------------------------------
  // 1. INPUT DETECTION
  // ------------------------------------------
  
  // switch shower
  bool nowShower = digitalRead(SWITCH_PIN_SHOWER);
  if (lastSwitchShower == HIGH && nowShower == LOW) {
    isShowering = !isShowering;  
    hasInteracted = true;        
  }
  lastSwitchShower = nowShower;

  // switch country
  bool nowCountry = digitalRead(SWITCH_PIN_COUNTRY);
  if (!isShowering && lastSwitchCountry == HIGH && nowCountry == LOW) {
    countryIndex = (countryIndex + 1) % 3;  // 0->1->2->0
    hasInteracted = true;        

    // --- PREVIEW FLASH ---
    // 1. Calculate the NEW target immediately just for the preview
    int p = (countryIndex == 0) ? RAIN_IRAN : (countryIndex == 1) ? RAIN_AUSTRIA : RAIN_INDONASIA;
    float previewLevel = (p * NUM_LEDS) / 100.0;

    // 2. Flash this level in WHITE so user sees what they picked
    FastLED.clear();
    for (int i = 0; i < (int)previewLevel; i++) {
       if (i < 3) leds[i] = CRGB::Red; // Keep bottom red
       else leds[i] = CRGB::White;     // Flash the rest WHITE
       if (i < 3) leds[i] = CRGB::Red; 
       else leds[i] = CRGB::White; // Flash White
    }
    FastLED.show();
    delay(400); // Pause briefly so they see the selection

    // 3. FORCE RESET TO 0
    // This guarantees the "Filling Animation" plays from the bottom,
    // even if the tank was already full.
    currentWaterLevel = 0; 
    lastTargetLevel = -1; // -1 forces the "Fast Transition" logic to trigger
  }
  lastSwitchCountry = nowCountry;


  // ------------------------------------------
  // 2. IDLE ANIMATION (Only before first interaction)
  // ------------------------------------------
  if (!hasInteracted) {
    if (currentSound != RAIN) {
       df.loop(1); 
       currentSound = RAIN;
    }
    analogWrite(VIBE_PIN, 0);
    analogWrite(SHOWER_LED_PIN, 0);

    // Idle Drip Physics
    if (currentMillis - lastDripMove > DRIP_SPEED) {
      lastDripMove = currentMillis;
      idleDripPos -= 1.0; 
      if (idleDripPos < 0) idleDripPos = NUM_LEDS - 1;
    }

    FastLED.clear();
    leds[(int)idleDripPos] = CRGB::Cyan; 
    FastLED.show();
    return; // Stop loop here
  }


  // ------------------------------------------
  // 3. NORMAL OPERATION
  // ------------------------------------------

  int percent = (countryIndex == 0) ? RAIN_IRAN : (countryIndex == 1) ? RAIN_AUSTRIA : RAIN_INDONASIA;
  float targetLevel = (percent * NUM_LEDS) / 100.0;

  // ==========================================
  // WATER LEVEL LOGIC
  // ==========================================
  if (isShowering) {
    dripPosition = -1;
    isRefilling = false;

    if (currentWaterLevel > 0) {
      currentWaterLevel -= DRAIN_SPEED;
      analogWrite(VIBE_PIN, 200);
      analogWrite(SHOWER_LED_PIN, 255); 

      if (currentSound != SHOWER) {
        df.loop(2); 
        currentSound = SHOWER;
      }
    } else {
      // Empty Tank Behavior
      analogWrite(VIBE_PIN, 0);
      analogWrite(SHOWER_LED_PIN, 0); 
      currentWaterLevel = 0;

      if (currentSound != EMPTY) {
        df.stop(); 
        currentSound = EMPTY;
      }
    }
  } else {
    // IDLE / REFILLING
    analogWrite(VIBE_PIN, 0);
    analogWrite(SHOWER_LED_PIN, 0); 

    // CASE A: Button was just pressed (Fast Animation)
    // We know this because lastTargetLevel was set to -1
    if ((int)targetLevel != lastTargetLevel) {
      if (currentWaterLevel < targetLevel) {
        currentWaterLevel += TRANSITION_SPEED; // FAST speed
        if (currentWaterLevel > targetLevel) currentWaterLevel = targetLevel;
      } 
      // (We don't need 'else' here because we reset to 0, so we always climb up)

      // Lock in the level when we get close
      if (abs(currentWaterLevel - targetLevel) < 0.1) {
        currentWaterLevel = targetLevel;
        lastTargetLevel = (int)targetLevel; // Animation Done
      }
      dripPosition = -1; 
      isRefilling = false;
    }
    // CASE B: Post-Shower Refill (Slow Rain)
    // This happens when target == lastTarget, but level is low
    else if (currentWaterLevel < targetLevel) {
      currentWaterLevel += 0.005; // SLOW speed
      isRefilling = true;
      if (dripPosition < 0) dripPosition = NUM_LEDS - 1; 
    } else {
      dripPosition = -1;
      isRefilling = false;
    }

    if (currentSound != RAIN) {
      df.loop(1);
      currentSound = RAIN;
    }
  }

  // ==========================================
  // DRIP MOVEMENT
  // ==========================================
  if (dripPosition >= 0 && currentMillis - lastDripMove > DRIP_SPEED) {
    lastDripMove = currentMillis;
    dripPosition -= 1.0; 
    if (dripPosition <= currentWaterLevel) {
      if (currentWaterLevel < targetLevel) {
        dripPosition = NUM_LEDS - 1; 
      } else {
        dripPosition = -1;
      }
    }
  }

  // ==========================================
  // DISPLAY
  // ==========================================
  FastLED.clear();
  int ledsToDisplay = (int)currentWaterLevel;
  for (int i = 0; i < ledsToDisplay; i++) {
    if (i < 3) leds[i] = CRGB::Red;
    else leds[i] = CRGB::Blue;
  }

  // White surface line during Fast Animation
  if ((int)targetLevel != lastTargetLevel && ledsToDisplay > 0 && ledsToDisplay <= NUM_LEDS) {
    leds[ledsToDisplay - 1] = CRGB::White; 
  }

  if (dripPosition >= 0) leds[(int)dripPosition] = CRGB::Cyan;
  if (isShowering && ledsToDisplay == 0) {
    if ((currentMillis / 250) % 2 == 0) fill_solid(leds, 3, CRGB::Red);
  }
  FastLED.show(); 
}