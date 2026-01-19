#include <FastLED.h>
#include <DFRobotDFPlayerMini.h>
#include <HardwareSerial.h>

// --- Configuration ---
#define POT_PIN     32      
#define VIBE_PIN    2       
#define SWITCH_PIN  13      

#define DATA_PIN    27      
#define CLOCK_PIN   14      
#define NUM_LEDS    41      
#define BRIGHTNESS  120     

// DFPlayer uses UART2: RX (GPIO 16), TX (GPIO 17) is standard for ESP32 UART2
#define RX_PIN      16 
#define TX_PIN      17 

HardwareSerial dfSerial(2); 
DFRobotDFPlayerMini df;
CRGB leds[NUM_LEDS];

// --- Settings ---
const int RAIN_IRAN = 30;    
const int RAIN_AUSTRIA = 65; 
const int RAIN_TAIWAN = 100; 
const float DRAIN_SPEED = 0.01;

// --- Variables ---
float currentWaterLevel = 0; 
int lastTargetLevel = 0;      
float dripPosition = -1;      
unsigned long lastDripMove = 0; 
const int DRIP_SPEED = 30; 

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
  dfSerial.begin(9600, SERIAL_8N1, RX_PIN, TX_PIN);
  delay(1000); 
  if (!df.begin(dfSerial)) {
    Serial.println("DFPlayer FAILED. Check wiring RX/TX and SD card.");
  } else {
    Serial.println("DFPlayer OK");
    df.volume(25); // 0-30
  }

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

  int percent = (potValue < 1365) ? RAIN_IRAN : (potValue < 2730) ? RAIN_AUSTRIA : RAIN_TAIWAN;
  float targetLevel = (percent * NUM_LEDS) / 100.0;

  // ==========================================
  // 1. WATER LEVEL & MOTOR & AUDIO STATE
  // ==========================================
  if (isShowering) {
    dripPosition = -1; 
    if (currentWaterLevel > 0) {
      currentWaterLevel -= DRAIN_SPEED;
      analogWrite(VIBE_PIN, 200);
      
      if (currentSound != SHOWER) {
        df.loop(2); // Play 0002.mp3 (Shower) in a loop
        currentSound = SHOWER;
      }
    } else {
      analogWrite(VIBE_PIN, 0); 
      currentWaterLevel = 0;
      
      if (currentSound != EMPTY) {
        df.play(3); // Play 0003.mp3 (Empty Warning)
        currentSound = EMPTY;
      }
    }
  } 
  else {
    analogWrite(VIBE_PIN, 0); 

    if (currentSound != RAIN) {
      df.loop(1); // Play 0001.mp3 (Ambient Rain) in a loop
      currentSound = RAIN;
    }

    if ((int)targetLevel != lastTargetLevel) {
      currentWaterLevel = targetLevel; 
      lastTargetLevel = (int)targetLevel;
      dripPosition = -1;
    } else if (currentWaterLevel < targetLevel) {
      currentWaterLevel += 0.005; 
      if (dripPosition < 0) dripPosition = NUM_LEDS - 1;
    } else {
      dripPosition = -1; 
    }
  }

  // ==========================================
  // 2. DRIP MOVEMENT & IMPACT SOUND
  // ==========================================
  if (dripPosition >= 0 && currentMillis - lastDripMove > DRIP_SPEED) {
    lastDripMove = currentMillis;
    dripPosition -= 1.0; 
    if (dripPosition <= currentWaterLevel) {
      // Impact Sound: We use play() for a one-shot sound
      // This will briefly interrupt the loop(1)
      df.play(4); 
      
      dripPosition = (currentWaterLevel < targetLevel) ? NUM_LEDS - 1 : -1;
    }
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
  if (dripPosition >= 0) leds[(int)dripPosition] = CRGB::Cyan;
  if (isShowering && ledsToDisplay == 0) {
    if ((currentMillis / 250) % 2 == 0) fill_solid(leds, 3, CRGB::Red);
  }
  FastLED.show();
}