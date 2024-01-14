# Reads and does *not* modify the fuse value that governs the RST pin's functionality.

# NOTE: If the lower nybble of the memory at Fuse offset 0x05 
# is not 0x04, then High Voltage UPDI (HV UPDI) will be strictly NECESSARY
# to program the board again. This is why the other script WriteATTinyResetFuse.ps1 carries a warning

# NOTE: Edit this to be your windows COM port for the debugger tool
#   You can find this opening the windows DeviceManager tool,
#   clicking on the dropdown for "Ports (COM and LPT)"
#   and then unplugging and replugging your device and see what COM port is toggled 
$comPort = "COM4"

pymcuprog read -m fuses -t uart -u $comPort -d attiny1606 -o 5
Write-Host -NoNewLine 'Press any key to continue...';
