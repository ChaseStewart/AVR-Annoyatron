# Programming the board

## Programming Interface
This project uses the [UPDI Programming Interface](https://onlinedocs.microchip.com/pr/GUID-DDB0017E-84E3-4E77-AAE9-7AC4290E5E8B-en-US-4/index.html?GUID-9B349315-2842-4189-B88C-49F4E1055D7F) in order to flash the ATTiny1606 with code changes. In this project, the TagConnect 6 pin No Legs footprint was used for programming to save board space and to avoid populating a big 2x3 header. 


## Hardware Needed
You will need the following to flash your board
* A USB cable
* A populated Annoyatron Board
* An [FTDI](https://www.adafruit.com/product/284)-to-[USB interface](https://www.sparkfun.com/products/9873) - Many options are available and counterfeit chips can be found, but Sparkfun and Adafruit are probably safe bets if you don't know what to get
* A [TagConnect TC2030-PKT-NL 6-pin connector](https://www.digikey.com/en/products/detail/tag-connect-llc/TC2030-PKT-NL/18713762)
* A [TagConnect Board-Retaining End Clip](https://www.digikey.com/en/products/detail/tag-connect-llc/TC2050-CLIP-3PACK/12318009)

**NOTE**: I have a probably-counterfeit FTDI debugger that works for most things including communicating over UART and flashing other types of boards, but that debugger just does not work with this UPDI- this is the reason for the warning in the FTDI-to-USB entry. 

## Steps and Images
First you will need to properly connect the FTDI debugger pins to the TagConnect cable, and connect the FTDI debugger to a USB cable. When that is done it should look like this:
![Image of the full programming tool including TagConnect cable](https://github.com/[username]/[reponame]/blob/main/Software/documentation/images/Full_Programmer_and_tagConnect.jpg?raw=true)

I wound up making a custom F-F Dupont-connector cable to ensure proper orientation (I use the black GND cable and red VCC to visually confirm orientation every time), 
but you can use jumper connectors or whatever you need. I populated the TXD, RXD, GND, and VCC pins only for this UPDI connector. 
![Image of just the FTDI-to-USB device and its connector cable](https://github.com/[username]/[reponame]/blob/main/Software/documentation/images/Programmer_UPDI_Cable.jpg?raw=true)

Now that you have the cable, you are going to look for this `TAG1` footprint on the board that looks like this. 
The feet of the TagConnect cable will go through the holes and then the little cap can be placed on the far side of the board to secure the cable. Spring tension will keep the pogo pins electrically connected the 6 circular pins on the face of the PCB. 
![Image of the location on the PCB where the TagConnect can be attached](https://github.com/[username]/[reponame]/blob/main/Software/documentation/images/TagConnect_Footprint.jpg?raw=true)

When you put it all together it should look like this:
![Image of the TagConnect cable connected to the board](https://github.com/[username]/[reponame]/blob/main/Software/documentation/images/Board_Connection.jpg?raw=true)

**NOTE:** When you have the board connected to a computer, **ENSURE THE 9-VOLT BATTERY IS REMOVED OR THE POWER SWITCH IS OFF**

Now use the scripts in the Scripts folder or set up a programming command in Microchip Studio and you should be able to flash your board
As a basic check, you can try to read the fuses with `ReadATTinyResetFuse.ps1`. If everything appears connected, but the UPDI check is failing, check your connector's pins to ensure TX, RX, VCC, and GND are properly connected, and you can also gently wiggle the tag-connect cable a bit and try again.