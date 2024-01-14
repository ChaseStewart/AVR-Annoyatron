# Programs your ATTiny with the binary pointed to by $ pathToFirmareHex

# NOTE: Edit this variable to point to your generated AnnoyatronFW.hex file
$pathToFirmwareHex = "$(MSBuildProjectDirectory)\Debug\AnnoyatronFW.hex"

# NOTE: Edit this to be your windows COM port for the debugger tool
#   You can find this opening the windows DeviceManager tool,
#   clicking on the dropdown for "Ports (COM and LPT)"
#   and then unplugging and replugging your device and see what COM port is toggled 
$comPort = COM4

pymcuprog write -t uart -d attiny1606 -u $comPort -f $pathToFirmwareHex -v info -c 57600 --verify