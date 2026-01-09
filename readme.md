# TODO List (please edit)
- [x] Write basic code and start breadboard prototype
- [x] Solder and test vibration motor
- [x] Solder and test switch
- [x] Find ways to power speaker and LED strip externally
- [x] Wire and test potentiometer
- [ ] Solder and wire LED strip
- [ ] Write test code for LED strip
- [ ] Drill holes for LED strip on pipes (when LED strip confirmed working)
- [ ] Test speaker with actual audio files (need library), may need more capable speaker
- [ ] Test potentiometer with capacitor, then perhaps replace pot with compatible one or a turning switch? (Issue with current pot: returns 0 for almost entire range of rotation, only last small rotation segment ramps up from 0 to 4095)
- [ ] Get and wire / solder blue LEDs for shower head
- [ ] Drill holes for LEDs on shower head
- [ ] Modify shower head to hold vibration motor and LEDs, fit cables through hose
- [ ] Build holder/platform for ESP32 and main electronics inside the pipes
- [ ] Buy spray-color (black?) for pipes, box
- [ ] Cut hole for bottom pipe into box (base)
- [ ] Modify umbrella (remove handle, ...)
- [ ] Decide on third country (Austria, Iran, ?) and come up with water values and timespans for showering experience
- [ ] Build fixture for holding the shower head when not in use --> needs to hold down the switch and release it when shower head gets picked up
- [ ] Complete breadboard prototype
- [ ] Complete housing (pipes, shower head, base, umbrella)
- [ ] Assemble and test
- [ ] Create 2 minute demo video (deadline 25th Jan)
- [ ] Create 1 A4 page scenario (deadline 25th Jan)
- [ ] Poster (?) for presentation on 28th Jan

![Dry Shower Prototype](/img/breadboard-prototype.jpeg)
![Power For LED Strip](/img/power-for-LED-strip.jpeg)

# Overview
The Dry Shower Project is an interactive IoT device built with an ESP32 microcontroller that simulates a "dry shower" experience. It uses vibration and sound to mimic the sensation of showering without water, inspired by varying rainfall levels in different countries. The device features a potentiometer for mode selection, a momentary toggle switch to activate the shower mode, a vibration motor for tactile feedback, and a speaker for ambient sounds.

# Features
- Mode Selection: Use a potentiometer to choose between three rainfall-inspired modes (buggy, see TODO)
- Toggle Switch Control: Momentary switch (3-position, springs back to neutral) activates the vibration motor only in the UP position (works)
- Vibration Feedback: Pulsing motor simulates water flow when active (works)
- Ambient Sounds: (may need more capable speaker, need to look into audio libraries)
    - Rain sounds (sparse droplets) when idle.
    - Shower sounds (dense water flow) when active.

# Wiring
ESP32 Pinout:
- Potentiometer: Wiper to GPIO34, one end to 3.3V, other to GND
- Toggle Switch: One terminal to GPIO4, other to GND (internal pull-up enabled)
- Vibration Motor: To GPIO2 via transistor
- Speaker/Buzzer: To GPIO25 (and GND)