# Watchy code by RevK

<img align=right width=25% src='Manuals/face.jpg'>

This is an ESP IDF based build of code for the Watchy. Hardware available from [SQFMI](https://watchy.sqfmi.com).

It includes menus on the watch for a choice of faces, time zones, WiFi setting, and over the air s/w updates. It inlcudes step counters.

It does not yet do the buzzer. It does have the good battery life you would expect (several weeks per charge).

Faces include:-

- Simple digital face
- Combined analogue/digital
- Analogue face
- Lunar (with moon phase, next full moon date/time)
- Death Star (like lunar but *that's no moon*)
- Minecraft (blocky text, etc)
- Coundown (to XMAS, days, hours, minutes), my grandkids love this one
- Alteran digits and random Stargate address

## Build/install

- Ensure you have [ESP IDF](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/) installed for ESP32
- Git clone with `--recursive` to get the submodules
- Connect USB to Watchy
- `make flash`

