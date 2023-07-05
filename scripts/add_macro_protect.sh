#!/bin/bash

directory=$1

if [ -z "$directory" ]; then
    echo "Usage: bash script.sh [directory]"
    exit 1
fi

if [ ! -d "$directory" ]; then
    echo "Directory not found."
    exit 1
fi

for file in "$directory"/*; do
    if [ -f "$file" ]; then
        name=$(basename "$file" | sed 's/\.cpp//g' |  sed 's/\.c//g' |sed 's/test//i' | tr '[:lower:]' '[:upper:]')
        temp_file="${file}.tmp"
        
        echo -e "#ifdef CPP_TEST_WITH${name}\n" > "$temp_file"
        cat "$file" >> "$temp_file"
        echo -e "\n#endif\n" >> "$temp_file"
        
        mv "$temp_file" "$file"
    fi
done
