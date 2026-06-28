#!/bin/bash
set -e

GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m'

BIN="/usr/local/bin/desktophotbar"
ICON_DIR="/usr/local/share/icons/hicolor/256x256/apps"
ICON="$ICON_DIR/desktophotbar.png"
DESKTOP="/usr/share/applications/DesktopHotbar.desktop"

uninstall() {
    echo -e "${YELLOW}Uninstalling DesktopHotbar...${NC}"
    sudo rm -f "$BIN" "$ICON" "$DESKTOP"
    echo -e "${GREEN}Done.${NC}"
}

if [ "$1" = "--uninstall" ]; then
    uninstall
    exit 0
fi

echo -e "${GREEN}=== DesktopHotbar Build & Install ===${NC}"

# check deps
for cmd in cmake g++; do
    if ! command -v $cmd &>/dev/null; then
        echo -e "${RED}Missing: $cmd. Install Qt6 build tools first.${NC}"
        exit 1
    fi
done
if ! pkg-config --exists Qt6Widgets 2>/dev/null; then
    echo -e "${RED}Qt6 not found.${NC}"
    echo "  Arch/CachyOS: sudo pacman -S qt6-base cmake gcc"
    echo "  Debian/Ubuntu: sudo apt install qt6-base-dev cmake g++"
    exit 1
fi

# build
echo "Building..."
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel $(nproc)
echo -e "${GREEN}Build done.${NC}"

# install
echo "Installing..."
sudo cp build/DesktopHotbar "$BIN"
sudo chmod +x "$BIN"

if [ -f icon.png ]; then
    # if hicolor is a file instead of a dir, remove it first
    sudo rm -f /usr/local/share/icons/hicolor 2>/dev/null || true
    sudo mkdir -p "$ICON_DIR"
    sudo cp icon.png "$ICON"
fi

sudo tee "$DESKTOP" >/dev/null << EOF
[Desktop Entry]
Name=Desktop Hotbar
Comment=Minecraft-style desktop hotbar
Exec=$BIN
Icon=desktophotbar
Terminal=false
Type=Application
Categories=Utility;
EOF

echo ""
echo -e "${GREEN}Installed.${NC}"
echo "  Run : $BIN"
echo "  Uninstall: ./install.sh --uninstall"
