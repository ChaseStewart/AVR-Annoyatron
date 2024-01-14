import wave  # Standard Python Library to deal with .WAV files


def parseWavFile(inFileName, outFileName, isWinNotLose):
    """Read an input wave file, write out a C-style header variable to feed into annoyatron Firmware

    Args:
        inFileName (string): filename and optionally path to an 8 bit wave file to convert
        outFileName (string): filename and optionally path where resulting C header should be written
        isWinNotLose (boolean): True = name resulting array in the header "youWin", else it's named "youLose"
 
    Returns:
        None
    """
    print('\nparsing ', inFileName)

    with wave.open(inFileName, "r") as w:
        framerate = w.getframerate()
        frames = w.getnframes()
        channels = w.getnchannels()
        width = w.getsampwidth()
        print('\tsampling rate:', framerate, 'Hz')
        print('\tlength:', frames, 'samples')
        print('\tchannels:', channels)
        print('\tsample width:', width, 'bytes')

        data = w.readframes(frames)

    with open("Outputs/"+outFileName, "w") as f:
        structname = "youWin" if isWinNotLose else "youLose";
        comment  = "/**\n"
        comment += " * AUTOGENERATED by Chase E Stewart for the AVR-Annoyatron\n"
        comment += " * copy the line below containing giant struct \"" + structname + "\" into audioArray.h,\n"
        comment += " * clobbering the existing struct already in audioArray.h\n"
        comment += " */\n"
        f.write(comment)
        struct_start = "const uint8_t " + structname + "[] = {"
        f.write(struct_start)

        outputarray = []
        for idx in range(len(data)):
            outputarray.append("0x{:02X}".format(data[idx]))

        f.write(", ".join(outputarray))
        f.write("};")

    print('\twrote output to: ', outFileName)


# Edit these to match your desired input- and output-filenames
WIN_INPUT_FILE = "YouWin.wav"
WIN_OUTPUT_FILE = "YouWinHeader.h"
WIN_IS_WIN = True

LOSE_INPUT_FILE = "YouLose.wav"
LOSE_OUTPUT_FILE = "YouLoseHeader.h"
LOSE_IS_WIN = False


# Now just run the script with reasonable defaults if this script
#   is invoked directly 
if __name__ == "__main__":
    print("Starting...")
    parseWavFile(WIN_INPUT_FILE, WIN_OUTPUT_FILE, WIN_IS_WIN)
    parseWavFile(LOSE_INPUT_FILE, LOSE_OUTPUT_FILE, LOSE_IS_WIN)
    print("Done!")