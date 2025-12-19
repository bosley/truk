#!/bin/bash

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
TARGET="${1:-cursor}"

if [ "$TARGET" != "cursor" ] && [ "$TARGET" != "vscode" ]; then
    echo "Usage: $0 [cursor|vscode]"
    exit 1
fi

if [ "$TARGET" = "cursor" ]; then
    EXT_DIR="$HOME/.cursor/extensions"
else
    EXT_DIR="$HOME/.vscode/extensions"
fi

LINK_PATH="${EXT_DIR}/truk-language"

echo "Installing Truk language extension to $TARGET..."

if [ ! -d "$EXT_DIR" ]; then
    echo "Error: Extensions directory not found: $EXT_DIR"
    exit 1
fi

echo "Removing old installations..."
rm -rf "${EXT_DIR}/truk-language"*
rm -rf "${EXT_DIR}/truk.truk-language"*

echo "Creating symlink: $LINK_PATH -> $SCRIPT_DIR"
ln -s "$SCRIPT_DIR" "$LINK_PATH"

echo ""
echo "✓ Installation complete!"
echo "✓ Symlinked to: $LINK_PATH"
echo ""
echo "Please restart $TARGET to activate syntax highlighting."
