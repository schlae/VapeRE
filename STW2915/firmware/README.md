# Puya PY32 MCU Vape Example Code

This code is written to exercise basic capabilities of the STW2915 board with a PY32F030.

It's derived from [py32-template](https://github.com/jaydcarlson/py32-template), so you'll probably want to head there to look at the readme and set up the prerequisites. 

It demonstrates the following functionality:

* Simple graphics on the LCD panel using hardware SPI and DMA for speed
* NOR flash access for storing a BMP file that can be blitted (in chunks) to the LCD
* ADC functions for measuring the on-chip temperature sensor, battery voltage, and USB voltage
* Power save mode (triggered by the pushbutton) with 5uA supply current
* A basic UART console, mainly for host access to the NOR flash. Badly-written Python scripts are provided in the utils directory for this.

## Building

For a command line build environment, make sure you have arm-none-eabi-gdb and the usual build tools.

## Debugging

Use pyOCD, although you will need to perform some extra steps to get the Puya PY32 SDK and find their DFP pack. See [py32-template](https://github.com/jaydcarlson/py32-template) for detailed instructions. It works well with my ST-LINK V2 clone and gdb-multiarch.

The STW2915, like most of these vapes, bring the SWD lines out through the USB-C charging port on the CC lines. I found a simple inline USB-C breakout board and connected it to my ST-LINK V2 clone as follows:

| USB-C pin | Name  | ST-LINK V2 pin |
|-----------|-------|----------------|
|    A5     | SWCLK | 2              |
|    B5     | SWDIO | 4              |
|    GND    | GND   | 6              |

You will need to try the breakout board in both orientations in case the pins are swapped.

To load your own firmware for the first time, you'll have to fight a bit with the power save modes which disable SWD access. I've found it's easiest to disconnect the battery and wire the +3.3V (pin 7 of the ST-LINK V2) to the former battery connection on the PCB. 

Then you'll have to act quickly to launch pyOCD and connect gdb-multiarch to it (maybe using the `-ex` switch to run the gdb commands as an argument) and run the `mon reset halt` command to take control before it times out and enters power save mode.

  
