#!/bin/bash

# Compile the program
echo "🔧 Compiling main.cpp..."
g++ main.cpp -o compressor
if [ $? -ne 0 ]; then
    echo "❌ Compilation failed."
    exit 1
fi
echo "✅ Compilation successful."

# Ask for operation mode
echo "Choose operation:"
echo "  1) Compress a file"
echo "  2) Decompress a file"
echo "  3) Run test"
read -p "Enter choice (1/2/3): " choice

if [ "$choice" == "1" ]; then
    read -p "Enter the name of the file to compress (e.g., data.csv): " input
    read -p "Enter the output compressed file name (e.g., data.huff): " output
    ./compressor -c "$input" "$output"
    echo "✅ Compression complete."

elif [ "$choice" == "2" ]; then
    read -p "Enter the name of the compressed file (e.g., data.huff): " input
    read -p "Enter the output decompressed file name (e.g., recovered.csv): " output
    ./compressor -d "$input" "$output"
    
    # Optional: Try verifying with original file
    orig="${input%.huff}.csv"
    if [ -f "$orig" ]; then
        echo "🔍 Checking if $orig matches $output..."
        if diff "$orig" "$output" > /dev/null; then
            echo "✅ Files match."
        else
            echo "❌ Files differ!"
        fi
    fi

elif [ "$choice" == "3" ]; then
    echo "🧪 Running automated test with sample input..."
    ORIGINAL="input.csv"
    COMPRESSED="compressed.huff"
    DECOMPRESSED="output.csv"

    echo "name,age\nAlice,30\nBob,25" > "$ORIGINAL"

    ./compressor -c "$ORIGINAL" "$COMPRESSED"
    ./compressor -d "$COMPRESSED" "$DECOMPRESSED"

    echo "🔍 Verifying test result..."
    if diff "$ORIGINAL" "$DECOMPRESSED" > /dev/null; then
        echo "✅ Test Passed: CSV integrity preserved."
    else
        echo "❌ Test Failed: Mismatch detected."
    fi

else
    echo "❌ Invalid choice. Exiting."
    exit 1
fi
