# Erases the program currently run by the ATTiny 
# It is recommended you do this every time you reprogram the board

# NOTE: Edit this to be your windows COM port for the debugger tool
#   You can find this opening the windows DeviceManager tool,
#   clicking on the dropdown for "Ports (COM and LPT)"
#   and then unplugging and replugging your device and see what COM port is toggled 
$comPort = "COM4"

pymcuprog erase -t uart -u $comPort -d attiny1606
Write-Host -NoNewLine 'Press any key to continue...';
$null = $Host.UI.RawUI.ReadKey('NoEcho,IncludeKeyDown');