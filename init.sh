#!/bin/bash

# 颜色定义
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m'

echo -e "${GREEN}=== 开始设置 DesktopHotbar 环境 ===${NC}"

if command -v python3 &> /dev/null; then
    PYTHON_CMD="python3"
elif command -v python &> /dev/null; then
    PYTHON_CMD="python"
else
    echo -e "${RED}错误: 系统中未找到 'python3' 或 'python' 命令。请先安装 Python 3。${NC}"
    exit 1
fi
echo -e "✅ Python 已找到，将使用命令: ${GREEN}$PYTHON_CMD${NC}"

VENV_DIR="venv"
if [ -d "$VENV_DIR" ]; then
    echo -e "${YELLOW}虚拟环境 '$VENV_DIR' 已存在，跳过创建步骤。${NC}"
else
    echo "▶️ 正在创建 Python 虚拟环境..."
    $PYTHON_CMD -m venv "$VENV_DIR"
    if [ $? -ne 0 ]; then
        echo -e "${RED}错误: 创建虚拟环境失败。${NC}"
        exit 1
    fi
    echo "✅ 虚拟环境创建成功。"
fi

echo "▶️ 正在激活虚拟环境并安装依赖..."
if [ ! -f "$VENV_DIR/bin/activate" ]; then
    echo -e "${RED}错误: 找不到虚拟环境的激活脚本。请检查 '$VENV_DIR' 目录是否完整。${NC}"
    exit 1
fi
source "$VENV_DIR/bin/activate"

if [ ! -f "requirements.txt" ]; then
    echo -e "${RED}错误: 'requirements.txt' 文件未找到。${NC}"
    deactivate
    exit 1
fi

pip install -r requirements.txt
if [ $? -ne 0 ]; then
    echo -e "${RED}错误: 安装依赖失败。${NC}"
    deactivate
    exit 1
fi
echo "✅ 依赖安装成功。"
echo "▶️ 正在生成桌面快捷方式..."

WORK_DIR=$(pwd)
PYTHON_EXEC_PATH="$WORK_DIR/$VENV_DIR/bin/python"
MAIN_PY_PATH="$WORK_DIR/main.py"

# 获取图标的绝对路径
if [ ! -f "icon.png" ]; then
    echo -e "${YELLOW}警告: 未在项目根目录找到 'icon.png'。快捷方式可能没有图标。${NC}"
    ICON_PATH=""
else
    ICON_PATH="$WORK_DIR/icon.png"
fi

DESKTOP_FILE_PATH="$HOME/Desktop/DesktopHotbar.desktop"
APPLICATION_FILE_PATH="/usr/share/applications/DesktopHotbar.desktop"
# 创建 .desktop 文件内容
cat > "$DESKTOP_FILE_PATH" << EOL
[Desktop Entry]
Version=1.0
Name=Desktop Hotbar
Comment=A Minecraft-style application hotbar for your desktop
Exec=$PYTHON_EXEC_PATH $MAIN_PY_PATH
Path=$WORK_DIR
Icon=$ICON_PATH
Terminal=false
Type=Application
Categories=Utility;
EOL

cat > "$APPLICATION_FILE_PATH" << EOL
[Desktop Entry]
Version=1.0
Name=Desktop Hotbar
Comment=A Minecraft-style application hotbar for your desktop
Exec=$PYTHON_EXEC_PATH $MAIN_PY_PATH
Path=$WORK_DIR
Icon=$ICON_PATH
Terminal=false
Type=Application
Categories=Utility;
EOL

if [ $? -ne 0 ]; then
    echo -e "${RED}错误: 创建 .desktop 文件失败。${NC}"
    deactivate
    exit 1
fi

echo "✅ 桌面文件已创建于: $DESKTOP_FILE_PATH 和 $DESKTOP_FILE_PATH"

为 .desktop 文件添加可执行权限
chmod +x "$DESKTOP_FILE_PATH"
echo "✅ 已为快捷方式添加可执行权限。"

# 退出虚拟环境
deactivate

echo -e "${GREEN}=== 全部设置完成！ ===${NC}"
echo "您现在可以从桌面双击 'Desktop Hotbar' 图标来启动程序了。"
