#!/bin/bash

# Configuration
PORT="/dev/ttyUSB0"

# The Fully Qualified Board Name (FQBN) for Arduino Nano
FQBN="arduino:avr:nano"

# NOTE: If you are using a cheaper clone Nano, they often use the old bootloader.
# If the upload fails, uncomment the line below instead:
# FQBN="arduino:avr:nano:cpu=atmega328old"

# Set this to the directory containing your .ino file ('.' means current directory)
SKETCH_DIR="." 

upload_logic() {
    echo "--- Arduino Nano detected on $PORT ---"
    # Give the USB-to-Serial chip a moment to fully initialize
    sleep 1 
    
    echo "Attempting upload..."
    
    # Upload the previously compiled code
    if ~/.local/bin/arduino-cli upload -p "$PORT" --fqbn "$FQBN" "$SKETCH_DIR"; then
        echo "✅ Upload Successful!"
    else
        echo "🚨 Upload failed."
        echo "Check connection, or if you have a clone Nano, try changing the FQBN to use the 'atmega328old' bootloader."
    fi
}

# Pre-compile so swapping boards is lightning fast
echo "Pre-compiling Arduino Nano sketch..."
if ~/.local/bin/arduino-cli compile --fqbn "$FQBN" "$SKETCH_DIR"; then
    echo "✅ Pre-compilation successful."
else
    echo "🚨 Compilation failed. Check your code."
    exit 1
fi

echo "Ready. Plug in your Arduino Nano..."

# Polling loop
while true; do
    if [ -e "$PORT" ]; then
        upload_logic
        
        echo "Waiting for device removal..."
        # Trap the script here until you unplug the board
        while [ -e "$PORT" ]; do 
            sleep 1 
        done
        echo "Device removed. Ready for next Arduino Nano."
    fi
    sleep 1
done
