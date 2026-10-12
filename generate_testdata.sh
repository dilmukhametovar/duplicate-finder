#!/usr/bin/bash

TARGET_DIR="./testdata"
NUM_FILES="10"
NUM_DUPLICATES="10"

rm -rf -- "$TARGET_DIR"
mkdir -p "$TARGET_DIR"

echo "Generating $NUM_FILES random files in $TARGET_DIR..."

for ((i = 1; i <= NUM_FILES; i++)); do
    size=$(( (RANDOM % 500 * 1024 + 1) * 1024 ))   # 1 KiB .. 500 MiB
    dd if=/dev/urandom of="$TARGET_DIR/file_${i}" \
       bs=1024 count=$(( size / 1024 )) status=none
done

echo "Copying $NUM_DUPLICATES files..."

mapfile -t files < <(find "$TARGET_DIR" -maxdepth 1 -type f -name 'file_*')

for ((i = 1; i <= NUM_DUPLICATES; i++)); do
    src="${files[$((RANDOM % ${#files[@]}))]}"
    cp "$src" "$TARGET_DIR/copy_${i}_$(basename "$src")"
done

echo "Done: $(find "$TARGET_DIR" -type f | wc -l) files created in $TARGET_DIR"
