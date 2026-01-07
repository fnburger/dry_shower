// --- Configuration ---
#define POT_PIN     34      // Potentiometer Pin
#define VIBE_PIN    2       // Vibration Motor Pin
#define SPEAKER_PIN 25      // Speaker/Buzzer Pin
#define SWITCH_PIN  4       // Toggle Switch Pin (change if using a different pin)

// --- Settings ---
const int PULSE_SPEED = 200; // Motor pulse speed in ms (lower = faster)
const int RAIN_IRAN = 30;    
const int RAIN_AUSTRIA = 65; 
const int RAIN_JAPAN = 95;   

// --- Variables ---
unsigned long lastPulseTime = 0;
bool motorState = false;

unsigned long lastSoundNote = 0;
int soundInterval = 100;

unsigned long lastSerialPrint = 0; // Timer for serial monitor

void setup() {
  Serial.begin(115200);
  Serial.println("--- DRY SHOWER TEST MODE ---");
  Serial.println("Motor: Pulsing (switch-controlled) | Sound: Rain (idle) or Shower (active) | Pot: Reading (averaged)...");
  
  pinMode(POT_PIN, INPUT);
  pinMode(VIBE_PIN, OUTPUT);
  pinMode(SWITCH_PIN, INPUT_PULLUP); // Enable internal pull-up for switch
  // pinMode(SPEAKER_PIN, OUTPUT); // Not required for tone()
}

void loop() {
  unsigned long currentMillis = millis();

  // ==========================================
  // 1. POTENTIOMETER, SWITCH DEBUG & SERIAL FEEDBACK
  // ==========================================
  // Check every 200ms for readable Serial Monitor
  if (currentMillis - lastSerialPrint > 200) {
    // Average 10 readings to smooth noise
    long potSum = 0;
    for (int i = 0; i < 10; i++) {
      potSum += analogRead(POT_PIN);
      delay(1); // Short delay between reads
    }
    int potValue = potSum / 10;

    String country = "";
    int waterLevel = 0;
    
    // Determine Country (adjust thresholds if remapping)
    if (potValue < 1365) {
      country = "Iran    "; // Spaces added for alignment
      waterLevel = RAIN_IRAN;
    } else if (potValue < 2730) {
      country = "Austria "; 
      waterLevel = RAIN_AUSTRIA;
    } else {
      country = "Japan   "; 
      waterLevel = RAIN_JAPAN;
    }
    
    // Read switch state for debug
    String switchState = (digitalRead(SWITCH_PIN) == LOW) ? "LOW (closed/UP?)" : "HIGH (open/neutral?)";
    
    // Print to Serial Monitor
    Serial.print("Raw/Avg Value: ");
    Serial.print(potValue);
    Serial.print("\t | Selected: ");
    Serial.print(country);
    Serial.print("\t | Start Level: ");
    Serial.print(waterLevel);
    Serial.print("%");
    Serial.print("\t | Switch state: ");
    Serial.println(switchState);
    
    lastSerialPrint = currentMillis;
  }

  // ==========================================
  // 2. PULSING MOTOR LOGIC (Switch-Controlled)
  // ==========================================
  bool switchUp = (digitalRead(SWITCH_PIN) == LOW); // LOW when UP (closed to GND)
  // If readings are inverted, uncomment: bool switchUp = (digitalRead(SWITCH_PIN) == HIGH);

  if (switchUp) {
    // Only pulse if switch is UP
    if (currentMillis - lastPulseTime >= PULSE_SPEED) {
      lastPulseTime = currentMillis;
      
      // Toggle Motor State
      if (motorState == false) {
        motorState = true;
        digitalWrite(VIBE_PIN, HIGH); // ON
      } else {
        motorState = false;
        digitalWrite(VIBE_PIN, LOW);  // OFF
      }
    }
  } else {
    // Switch neutral: Force motor OFF
    digitalWrite(VIBE_PIN, LOW);
    motorState = false; // Reset state for next toggle
  }

  // ==========================================
  // 3. AMBIENT SOUND (Rain or Shower based on motor activity)
  // ==========================================
  if (currentMillis - lastSoundNote > soundInterval) {
    // Determine sound mode: Rain if idle (no switchUp), Shower if active (switchUp)
    bool isShowering = switchUp;
    
    int freqMin, freqMax, duration, intervalMin, intervalMax;
    
    if (!isShowering) {
      // Rain: Sparse, low-frequency droplets
      freqMin = 100;
      freqMax = 600;
      duration = 50;
      intervalMin = 50;
      intervalMax = 150;
    } else {
      // Shower: Denser, higher-frequency for water flow
      freqMin = 800;
      freqMax = 2000;
      duration = 30;
      intervalMin = 20;
      intervalMax = 60;
    }
    
    // Play the tone
    tone(SPEAKER_PIN, random(freqMin, freqMax + 1), duration);  // +1 to include max
    
    lastSoundNote = currentMillis;
    soundInterval = random(intervalMin, intervalMax + 1);  // +1 to include max
  }
}