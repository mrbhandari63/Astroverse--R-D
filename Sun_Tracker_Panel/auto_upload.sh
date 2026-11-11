#!/bin/bash

# Configuration
PORT="/dev/ttyACM1"

# The Fully Qualified Board Name (FQBN) for Arduino UNO
FQBN="arduino:avr:uno"

# Set this to the directory containing your .ino file ('.' means current directory)
SKETCH_DIR="." 

upload_logic() {
    echo "--- Arduino UNO detected on $PORT ---"
    # Give the USB-to-Serial chip a moment to fully initialize
    sleep 1 
    
    echo "Attempting upload..."
    
    # Upload the previously compiled code
    if ~/.local/bin/arduino-cli upload -p "$PORT" --fqbn "$FQBN" "$SKETCH_DIR"; then
        echo "✅ Upload Successful!"
    else
        echo "🚨 Upload failed with standard bootloader."
        echo "🔄 Retrying with 'Old Bootloader' (Nano clone fallback)..."
        
        # Fallback to Nano with Old Bootloader
        if ~/.local/bin/arduino-cli upload -p "$PORT" --fqbn "arduino:avr:nano:cpu=atmega328old" "$SKETCH_DIR"; then
            echo "✅ Upload Successful using Old Bootloader!"
        else
            echo "🚨 Fallback upload failed too."
            echo "Check connection and try again."
        fi
    fi
}

# Pre-compile so swapping boards is lightning fast
echo "Pre-compiling Arduino UNO sketch..."
if ~/.local/bin/arduino-cli compile --fqbn "$FQBN" "$SKETCH_DIR"; then
    echo "✅ Pre-compilation successful."
else
    echo "🚨 Compilation failed. Check your code."
    exit 1
fi

echo "Ready. Plug in your Arduino UNO..."

# Polling loop
while true; do
    if [ -e "$PORT" ]; then
        upload_logic
        
        echo "Waiting for device removal..."
        # Trap the script here until you unplug the board
        while [ -e "$PORT" ]; do 
            sleep 1 
        done
        echo "Device removed. Ready for next Arduino UNO."
    fi
    sleep 1
done
