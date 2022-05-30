# Bluetooth-Calipers

Sends caliper data to connected bluetooth device when button is pressed.

Includes custom 3D files (STEP, 3mf, and STL) that provide a right angle connector for the connector and button as well as a case for the ESP32. 

I have only tested with iGuaging calipers (or other measuring devices) that have a 5-pin data port. This should also work with Mitutoyo devices but I have not personally tested. 

Supports two types of button presses: 
1. **Short press**: Data is sent without pressing the return key (used when you want to modify the value)
2. **Long press**: Data is sent and the enter key is pressed (used when the value does not need to be modified)

## Required hardware
1. ESP32 - I used [this development board from amazon](https://www.amazon.com/HiLetgo-ESP-WROOM-32-Development-Microcontroller-Integrated/dp/B0718T232Z/ref=sr_1_4?crid=5GS42RL0ZTBY&keywords=esp32&qid=1653872753&sprefix=esp32%2Caps%2C75&sr=8-4)
2. Button - I used a 6mm x 6mm x 3.1mm button [from this kit on amazon](https://www.amazon.com/gp/product/B07LCBLB8N/ref=ppx_yo_dt_b_asin_title_o08_s00?ie=UTF8&psc=1)
3. 2N2222 Transistor 
4. 100k ohm resistor
5. Solid core wire (I tested with internal wires from solid core ethernet cables and breadboard jumper cables)
6. Wire/solder
7. 3D printer

## External Sources
Number 1 was used heavily for diagrams and base code (useful when you want to constantly return a value via USB without a button press). 
Number 2 has additional schematics and links to doccumentions provided by mitutoyo 

1. https://www.instructables.com/Interfacing-a-Digital-Micrometer-to-a-Microcontrol/
2. https://github.com/Roger-random/mitutoyo
