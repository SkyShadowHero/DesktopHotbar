<p align="center"><a href="README.md">English</a> | 中文</a></p>

# DesktopHotbar

Minecraft 风格桌面物品栏，**Qt6/C++** 构建，支持 X11 和 Wayland。

原 PyQt5 版本保留在 `main.py`，可按需使用。

## 预览
![view](view.png)

## 功能

- 拖放 `.desktop` 文件到 9 个物品栏格子
- 左键点击启动应用（通过 `gio launch`，与桌面环境行为一致）
- 右键菜单（Minecraft 书本风格，book.png 背景）
  - 启动 / 从物品栏移除 / 单格设置 / 总设置
- 总设置：窗口位置锁定、缩放比例（0.25x ~ 10x）
- 鼠标悬停高亮
- 配置文件持久化（`~/.config/desktophotbar/config.json`）
- X11 窗口位置记忆，Wayland 下由 compositor 管理

## 依赖

| 发行版 | 安装命令 |
|--------|----------|
| Arch / CachyOS / Manjaro | `sudo pacman -S qt6-base cmake gcc` |
| Debian / Ubuntu / Deepin | `sudo apt install qt6-base-dev cmake g++` |
| Fedora | `sudo dnf install qt6-qtbase-devel cmake gcc-c++` |

输入法支持：`fcitx5-qt6`（可选）

## 编译 & 运行

```shell
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel $(nproc)
./build/DesktopHotbar
```

## 安装

```shell
./install.sh               # 编译 + 安装到系统
./install.sh --uninstall   # 卸载
```

## Python 版本（备用）

```shell
pip install -r requirements.txt
python main.py
```

## 素材

由 [Mc Assets](https://mcasset.cloud/) 提供
