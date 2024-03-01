#!/bin/bash

set -e

# Set directory path (modify if needed)
dir="examples"

# Loop through each file in the directory
for file in "$dir"/*; do
  # Check if it's a regular file (avoid hidden files, etc.)
  if [[ -f "$file" ]]; then
    # Execute "amVy" on the file
    if ! "./amVy" "$file" &> /dev/null; then
      echo "error: amVy failed on file: $file"
      exit 1
    fi
  fi
done

