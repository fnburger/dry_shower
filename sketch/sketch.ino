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

// DFPlayer uses UART2: RX (GPIO 16), TX (GPIO 17)
#define RX_PIN 16
#define TX_PIN 17

HardwareSerial dfSerial(2);
DFRobotDFPlayerMini df;
CRGB leds[NUM_LEDS];

// --- Settings ---
const int RAIN_IRAN = 10;
const int RAIN_AUSTRIA = 46;
const int RAIN_INDONASIA = 100;

// Speeds
const float DRAIN_SPEED = 0.005;      // Speed when showering
const float FILL_SPEED = 0.01;       // Approx 15 seconds to fill full
const int DRIP_SPEED = 20;           // Faster falling drops for rain effect

// --- Variables ---
float currentWaterLevel = 0;
float fillDripPos = -1;              // Position of the drop falling during fill
unsigned long lastDripMove = 0;

bool isShowering = false;
bool lastSwitchShower = HIGH;
int countryIndex = 0; 
bool lastSwitchCountry = HIGH;

// State Logic
bool hasInteracted = false;          
unsigned long lastInputTime = 0;     
bool collectionStarted = false;      
float idleDripPos = NUM_LEDS - 1;    

// NEW: Empty State Timer
unsigned long emptyTimerStart = 0;   

// Audio State Tracking
enum SoundState { SILENT, RAIN, SHOWER, EMPTY };
SoundState currentSound = SILENT;

void setup() {
  Serial.begin(115200);
  FastLED.addLeds<WS2801, DATA_PIN, CLOCK_PIN, RGB>(leds, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);
  FastLED.clear();
  FastLED.show();

  // DFPlayer Init
  delay(3000);
  dfSerial.begin(9600, SERIAL_8N1, RX_PIN, TX_PIN);
  
  if(df.begin(dfSerial)) {
    delay(200);
    df.reset();
    delay(1000); 
    df.volume(30);
  }

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
  
  // -- SHOWER SWITCH --
  bool nowShower = digitalRead(SWITCH_PIN_SHOWER);
  if (lastSwitchShower == HIGH && nowShower == LOW) {
    if (currentWaterLevel > 0 || isShowering) {
      isShowering = !isShowering;  
    }
    hasInteracted = true;        
    lastInputTime = currentMillis; 
    emptyTimerStart = 0; // Cancel empty timer if user presses button
  }
  lastSwitchShower = nowShower;

  // -- COUNTRY SWITCH --
  bool nowCountry = digitalRead(SWITCH_PIN_COUNTRY);
  if (!isShowering && lastSwitchCountry == HIGH && nowCountry == LOW) {
    countryIndex = (countryIndex + 1) % 3; 
    
    // Reset Logic
    hasInteracted = true;        
    lastInputTime = currentMillis;  
    collectionStarted = false;      
    currentWaterLevel = 0;          
    fillDripPos = NUM_LEDS - 1;     
    emptyTimerStart = 0; // Cancel empty timer if user presses button
  }
  lastSwitchCountry = nowCountry;


  // ------------------------------------------
  // 2. ATTRACT MODE (Idle State)
  // ------------------------------------------
  if (!hasInteracted) {
    if (currentSound != RAIN) { df.loop(1); currentSound = RAIN; }
    analogWrite(VIBE_PIN, 0);
    analogWrite(SHOWER_LED_PIN, 0);

    // Falling Drop Animation
    if (currentMillis - lastDripMove > DRIP_SPEED) {
      lastDripMove = currentMillis;
      idleDripPos -= 1.0; 
      if (idleDripPos < 0) idleDripPos = NUM_LEDS - 1;
    }

    FastLED.clear();
    leds[(int)idleDripPos] = CRGB::Cyan; 
    FastLED.show();
    return; // STOP LOOP HERE
  }


  // ------------------------------------------
  // 3. MAIN LOGIC
  // ------------------------------------------

  int percent = (countryIndex == 0) ? RAIN_IRAN : (countryIndex == 1) ? RAIN_AUSTRIA : RAIN_INDONASIA;
  float targetLevel = (percent * NUM_LEDS) / 100.0;

  // --- A. SHOWERING BEHAVIOR ---
  if (isShowering) {
    fillDripPos = -1; // Stop raining
    
    if (currentWaterLevel > 0) {
      currentWaterLevel -= DRAIN_SPEED;
      analogWrite(VIBE_PIN, 200);
      analogWrite(SHOWER_LED_PIN, 255); 
      if (currentSound != SHOWER) { df.loop(2); currentSound = SHOWER; }
    } else {
      // === TANK IS EMPTY ===
      analogWrite(VIBE_PIN, 0);
      analogWrite(SHOWER_LED_PIN, 0); 
      currentWaterLevel = 0;
      isShowering = false; 
      
      // Start the 10-second Empty Timer
      emptyTimerStart = currentMillis; 
      
      if (currentSound != EMPTY) { df.stop(); currentSound = EMPTY; }
    }
  } 
  // --- B. EMPTY STATE (10s Delay) ---
  else if (emptyTimerStart > 0) {
      // We are in the 10s delay period.
      // Do nothing logic-wise, just wait.
      // If 10s passes, go to Idle.
      if (currentMillis - emptyTimerStart > 10000) {
          hasInteracted = false;    // Go to Idle Mode
          emptyTimerStart = 0;      // Reset timer
          collectionStarted = false; 
      }
      // Note: Red blink is handled in Rendering section
  }
  // --- C. NORMAL FILLING BEHAVIOR ---
  else {
    analogWrite(VIBE_PIN, 0);
    analogWrite(SHOWER_LED_PIN, 0); 

    // CHECK 5-SECOND TIMER
    if (!collectionStarted) {
      if (currentMillis - lastInputTime > 5000) {
        collectionStarted = true; 
        fillDripPos = NUM_LEDS - 1; // Start the rain from top
      }
    }

    // FILL & RAIN ANIMATION
    if (collectionStarted) {
      if (currentWaterLevel < targetLevel) {
        currentWaterLevel += FILL_SPEED; 
        if (currentWaterLevel > targetLevel) currentWaterLevel = targetLevel;
      }

      // Rain Physics
      if (currentWaterLevel < targetLevel) {
         if (currentMillis - lastDripMove > DRIP_SPEED) {
            lastDripMove = currentMillis;
            fillDripPos -= 1.0; 
            if (fillDripPos <= currentWaterLevel) fillDripPos = NUM_LEDS - 1; 
         }
      } else {
         fillDripPos = -1; 
      }

    } else {
      currentWaterLevel = 0;
      fillDripPos = -1;
    }

    if (currentSound != RAIN) { df.loop(1); currentSound = RAIN; }
  }


  // ------------------------------------------
  // 4. LED RENDERING
  // ------------------------------------------
  FastLED.clear();

  int ledsToDisplay = (int)currentWaterLevel;

  // Draw Water (Blue/Red)
  for (int i = 0; i < ledsToDisplay; i++) {
    if (i < 3) leds[i] = CRGB::Red;
    else leds[i] = CRGB::Blue;
  }

  // Draw "Selection Marker" (Only during Selection Phase)
  if (!collectionStarted && !isShowering && emptyTimerStart == 0) {
    int targetIndex = (int)targetLevel - 1;
    if(targetIndex >= 0 && targetIndex < NUM_LEDS) {
      leds[targetIndex] = CRGB::White; 
    }
  }

  // Draw White Surface Line
  if (collectionStarted && currentWaterLevel < targetLevel && ledsToDisplay > 0) {
     leds[ledsToDisplay - 1] = CRGB::White;
  }

  // Draw Rain Drop
  if (fillDripPos >= 0) {
     leds[(int)fillDripPos] = CRGB::Cyan;
  }

  // BLINK RED: When Showering empty OR during the 10s Empty Delay
  if ((isShowering && ledsToDisplay == 0) || emptyTimerStart > 0) {
    if ((currentMillis / 250) % 2 == 0) fill_solid(leds, 3, CRGB::Red);
  }

  FastLED.show(); 
}