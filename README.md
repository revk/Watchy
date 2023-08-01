# Watchy code by RevK

<img align=right width=25% src='Manuals/face.jpg'>

This is an ESP IDF basd build of code for the Watchy, available from [SQFMI](https://watchy.sqfmi.com).

It includes menus on the watch for a choice of faces, time zones, WiFi setting, and over teh air s/w updates. It inlcudes step counters.

It does not yet do the buzzer, and does not do weather app stuff. It does have the good battery life you woudl expect (several weeks per charge).

## Build/install

- Git clone with `--recursive` to get the submodules
- Connect USB to Watchy
- `make flash`

