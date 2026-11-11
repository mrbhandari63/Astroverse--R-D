#!/bin/bash

# Configuration
SKETCH_PATH="."
# FQBNs for both bootloader versions
NEW_BOOT="arduino:avr:nano"
OLD_BOOT="arduino:avr:nano:cpu=atmega328old"

upload_logic() {
    echo "--- Device detected on $PORT ---"
    sleep 1 # Wait for device to initialize
    
    echo "Attempting upload with New Bootloader (Timeout: 6s)..."
    
    # timeout 6s will kill the process if it hangs on the 10x avrdude loop
    if timeout 6s arduino-cli upload -p $PORT --fqbn $NEW_BOOT $SKETCH_PATH; then
        echo "✅ Upload Successful (New Bootloader)"
    else
        echo "⏳ Timed out or failed. Switching to Old Bootloader..."
        
        # Give the port a second to release from the killed process
        sleep 1 
        
        if arduino-cli upload -p $PORT --fqbn $OLD_BOOT $SKETCH_PATH; then
            echo "✅ Upload Successful (Old Bootloader)"
        else
            echo "🚨 Both attempts failed. Check connections, bootloader, or cable."
        fi
    fi
}

# Compile once at the start to save time
echo "Compiling sketch..."
arduino-cli compile --fqbn $NEW_BOOT $SKETCH_PATH

echo "Ready. Plug in your Nano..."

# Check if there is already a device connected
for dev in /dev/ttyUSB* /dev/ttyACM*; do
    if [ -e "$dev" ]; then
        PORT="$dev"
        upload_logic
        echo "Waiting for device removal..."
        while [ -e "$PORT" ]; do sleep 0.5; done
        echo "Device removed. Ready for next Nano."
        break # We only handle one at a time for the initial check
    fi
done

# Watch /dev for the appearance of any ttyUSB or ttyACM device
inotifywait -m -q -e create /dev/ --format "%f" | while read -r filename; do
    if [[ "$filename" == ttyUSB* ]] || [[ "$filename" == ttyACM* ]]; then
        PORT="/dev/$filename"
        upload_logic
        echo "Waiting for device removal..."
        # Wait until the device is unplugged before watching again
        while [ -e "$PORT" ]; do sleep 0.5; done
        echo "Device removed. Ready for next Nano."
    fi
done
