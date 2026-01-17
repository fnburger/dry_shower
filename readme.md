WARNING: GPIO PIN 4 ON THE PROVIDED ESP32 IS BROKEN. GPIO PIN 34 IS NOT STABLE ENOUGH FOR POTENTIOMETER
# TODO List (please edit)
- [x] Write basic code and start breadboard prototype
- [x] Solder and test vibration motor
- [x] Solder and test switch
- [x] Find ways to power speaker and LED strip externally
- [x] Wire and test potentiometer
- [x] Solder and wire LED strip
- [x] Write test code for LED strip
- [x] Drill holes for LED strip on pipes (when LED strip confirmed working)
- [ ] Test speaker with actual audio files (need library), may need more capable speaker / louder speaker
- [x] (SOLVED) Test potentiometer with capacitor, then perhaps replace pot with compatible one or a turning switch? (Issue with current pot: returns 0 for almost entire range of rotation, only last small rotation segment ramps up from 0 to 4095)
- [x] Get and wire / solder blue LEDs for shower head
- [ ] Drill holes for LEDs on shower head or 3D print cover
- [x] Modify shower head to hold vibration motor and LEDs, fit cables through hose
- [ ] Build holder/platform for ESP32 and main electronics inside the pipes
- [ ] Buy spray-color (black?) for pipes, box
- [x] Cut hole for bottom pipe into box (base)
- [ ] Modify umbrella (remove handle, ...)
- [ ] Decide on third country (Austria, Iran, ?) and come up with water values and timespans for showering experience
- [ ] Build fixture for holding the shower head when not in use --> needs to hold down the switch and release it when shower head gets picked up
- [x] Complete breadboard prototype
- [ ] Complete housing (pipes, shower head, base, umbrella)
- [ ] Assemble and test
- [ ] Create 2 minute demo video (deadline 25th Jan)
- [ ] Create 1 A4 page scenario (deadline 25th Jan)
- [ ] Poster for presentation on 28th Jan

![Dry Shower Prototype](/img/LED_strip.jpeg)
![Power For LED Strip](/img/wiring.jpeg)

# Overview
The Dry Shower Project is an interactive exhibition device built with an ESP32 microcontroller that simulates a "dry shower" experience. It uses vibration, lights and sound to mimic the sensation of showering with limited water, inspired by varying rainfall levels in different countries. The device features a potentiometer for mode selection, a toggle switch to activate the shower mode, a vibration motor for tactile feedback, an LED strip to simulate the current remaining water level, LEDs on the shower head to visualize water coming out, and a speaker for ambient sounds.

# Features
- Mode Selection: Use a potentiometer to choose between three rainfall-inspired modes ()
- LED Strip (works)
- LED on the shower head (needs to be assembled and tested)
- Toggle Switch Control: Switch (springs back to neutral) activates the vibration motor only in the UP position (works)
- Vibration Feedback: Pulsing motor simulates water flow when active (works)
- Ambient Sounds: (may need more capable speaker, need to look into audio libraries or replace this with mechanical sounds)
    - Rain sounds (sparse droplets) when idle.
    - Shower sounds (dense water flow) when active.

# Wiring
ESP32 Pinout:
- Potentiometer: Wiper to GPIO32, one end to 3.3V, other to GND
- Toggle Switch: One terminal to GPIO13, other to GND (internal pull-up enabled)
- Vibration Motor: To GPIO2 via transistor
- Speaker/Buzzer: To GPIO25 (and GND), external 5V power supply
- LED Strip: DI to GPIO 27, CI to GPIO 14, external 5V and GND (connect external GND to ESP32 GND as well)
- LED (Shower head): tbd