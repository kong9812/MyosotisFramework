#!/bin/bash
# Copyright (c) 2025 kong9812

SHADER_DIR="$(pwd)/core/resources/shaders"

OUTPUT_DIR="$SHADER_DIR/spv"
[ -d "$OUTPUT_DIR" ] || mkdir -p "$OUTPUT_DIR"

echo "Converting shaders in $SHADER_DIR..."

find "$SHADER_DIR" -type f \( -name "*.vert" -o -name "*.frag" -o -name "*.comp" -o -name "*.task" -o -name "*.mesh" -o -name "*.rgen" -o -name "*.rchit" -o -name "*.rmiss" \) | while read -r f; do
    echo "Converting: $f"
    glslangValidator -V --target-env vulkan1.3 "$f" -o "$OUTPUT_DIR/$(basename "$f").spv"
done

echo "Shader conversion completed!"

if [ -z "$SKIP_PAUSE" ]; then
    read -p "Press [Enter] key to continue..."
fi