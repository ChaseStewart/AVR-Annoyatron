# WARNING WARNING WARNING Please Read WARNING WARNING WARNING

# This script destructively modifies the fuses!
# It STOPS NORMAL ABILITY TO PROGRAM THE ATTINY1606 AFTER ONE USE!

# It is useful, however, as this fuse burn must be applied in order for the 
# RESET button to be functional. The Reset button is also used for UPDI programming
# so the user must decide which functionality they wish to use. 
# The proper flow is program the board, determine it is working fully satisfactorily,
# then burn this fuse to enable the RESET button's use and disable further programming

# This script writes the RSTPINCFG bitfield in the SYSCFG0 Fuse (offset 0x05) of the ATTiny1606
# In order to change the Reset/UPDI pin from UPDI usage to RESET. The board is already designed
# to otherwise support RESET, and the RESET button will just begin working as expected after this is done

pymcuprog write -m fuses -t uart -u COM4 -d attiny1606 -o 5 -l 0xC8
Write-Host -NoNewLine 'Press any key to continue...';