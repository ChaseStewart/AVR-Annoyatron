# AVR-Annoyatron
Rebuilding an undergrad project to reflect new knowledge and skills on a different MCU

# Author
[Chase E. Stewart](https://chasestewart.co) for [Hidden Layer Design](https://hiddenlayerdesign.com)

# Repository Layout
`/Hardware/Images/...` - Embeddable images of the schematic and board layout

`/Hardware/Board_Files/...` - Gerber files that can be used to order the circuit board

`/Software/AnnoyatronFW/...` - Firmware for the Annoyatron project, in the form of a Microchip Studio project

# Setup Instructions

## Hardware
* Order the board from a PCB manufacturer using the Board_Files
* Order the parts from an electronics company using the BOM
* Assemble the PCB

## Software
* Download [Python 3.10](https://www.python.org/) or similar if you don't have it (don't go too far back) and put it in your executable path
* Download [pymcuprog](https://pypi.org/project/pymcuprog/) using pip or whatever works
* Download [Microchip Studio](https://www.microchip.com/en-us/tools-resources/develop/microchip-studio) and load the project from `/Software/AnnoyatronFw/...`
* Set up the custom programming tool in Microchip Studio to suit your configuration 
  - Mine was `pymcuprog write -t uart -d attiny406 -u COM12 -f "$(MSBuildProjectDirectory)\Debug\AnnoyatronFW.hex" -v info -c 57600 --verify`
* Acquire a Tag-Connect programming tool and connect it to a COM-to-serial USB dongle to do the flashing 
  - *NOTE: Don't install a battery until you've successfully flashed the code and the board appears to be working over USB power!!*
* You will need to use a `pymcuprog` erase command between every flash to make sure your program will be successfully verified
  - Mine was `pymcuprog erase -t uart -u COM12 -d attiny406`

## Running the program
* Remove the programming header and install a 9-volt battery
* Turn on the power using the switch
* Move or wave to trigger the PIR sensor, and then cut the proper wire before time runs out

# Errata
* The MCU for this project will soon change to attiny1606, *this README.md and the code will need to change afterwards*
