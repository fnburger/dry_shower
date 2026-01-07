# Overview
The Dry Shower Project is an interactive IoT device built with an ESP32 microcontroller that simulates a "dry shower" experience. It uses vibration and sound to mimic the sensation of showering without water, inspired by varying rainfall levels in different countries. The device features a potentiometer for mode selection, a momentary toggle switch to activate the shower mode, a vibration motor for tactile feedback, and a speaker for ambient sounds.

# Features
- Mode Selection: Use a potentiometer to choose between three rainfall-inspired modes.
- Toggle Switch Control: Momentary switch (3-position, springs back to neutral) activates the vibration motor only in the UP position.
- Vibration Feedback: Pulsing motor simulates water flow when active.
- Ambient Sounds:
    - Rain sounds (sparse droplets) when idle.
    - Shower sounds (dense water flow) when active.

# Wiring
ESP32 Pinout:
- Potentiometer: Wiper to GPIO34, one end to 3.3V, other to GND
- Toggle Switch: One terminal to GPIO4, other to GND (internal pull-up enabled)
- Vibration Motor: To GPIO2 via transistor
- Speaker/Buzzer: To GPIO25 (and GND)