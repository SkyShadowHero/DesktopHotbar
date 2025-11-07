import sys
import os
import json
import subprocess
from pathlib import Path
import shutil

from PyQt5.QtWidgets import (QApplication, QMainWindow, QWidget, QLabel,
                           QMessageBox, QMenu, QAction, QDialog, 
                           QVBoxLayout, QLineEdit, QDialogButtonBox,
                           QCheckBox, QComboBox, QHBoxLayout)
from PyQt5.QtCore import Qt, QPoint, QSize, pyqtSignal
from PyQt5.QtGui import QPixmap, QScreen, QIcon

try:
    from PyQt5.Qt import QIcon
except ImportError:
    pass



def resource_path(relative_path):
    try:
        base_path = sys._MEIPASS
    except Exception:
        base_path = os.path.abspath(".")
    return os.path.join(base_path, relative_path)

def setup_fcitx5_im_plugin():
    """
    注入Fcitx5输入法插件，确保在开发和打包环境下都能输入中文。
    """
    # 此功能仅在Linux上需要
    if sys.platform != "linux":
        return

    try:
        import PyQt5
        # 确定目标位置：PyQt5环境中的插件目录
        pyqt_path = Path(PyQt5.__file__).parent
        target_plugin_dir = pyqt_path / "Qt5" / "plugins" / "platforminputcontexts"
        target_plugin_file = target_plugin_dir / "libfcitx5platforminputcontextplugin.so"

        source_plugin_file = Path(resource_path("lib/libfcitx5platforminputcontextplugin.so"))

        # 如果源文件不存在（打包时忘加，或本地没有），则直接返回
        if not source_plugin_file.exists():
            print(f"输入法插件源文件未找到: {source_plugin_file}")
            return

        # 如果目标文件已经存在，说明环境无需处理，直接返回
        if target_plugin_file.exists():
            return

        # 执行注入：创建目标目录并复制插件文件
        print(f"检测到Fcitx5插件缺失，正在尝试注入...")
        target_plugin_dir.mkdir(parents=True, exist_ok=True)
        shutil.copy(source_plugin_file, target_plugin_file)
        print(f"成功将 {source_plugin_file.name} 复制到 {target_plugin_dir}")

    except Exception as e:
        # 捕获任何可能的错误，避免程序因此崩溃
        print(f"自动注入 Fcitx5 插件时发生错误: {e}")

MODERN_LIGHT_STYLE = """
    /* --- 基础窗口和字体 --- */
    QDialog, QWidget {
        background-color: #F5F5F5;
        color: #212121;
        font-size: 14px;
        font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, "Helvetica Neue", Arial, sans-serif;
    }

    /* --- 标签 --- */
    QLabel {
        color: #212121;
        padding-top: 5px;
    }

    /* --- 输入框和下拉框 --- */
    QLineEdit, QComboBox {
        background-color: #FFFFFF;
        color: #212121;
        border: 1px solid #DCDCDC;
        border-radius: 5px;
        padding: 8px;
        min-height: 20px;
    }
    QLineEdit:focus, QComboBox:focus {
        border: 1px solid #0078D7;
    }

    /* --- 下拉框箭头 --- */
    QComboBox::drop-down {
        border: none;
        width: 20px;
    }
    QComboBox::down-arrow {
        image: url(none);
    }

    /* --- 下拉列表容器 --- */
    QComboBox QAbstractItemView {
        background-color: #FFFFFF;
        border: 1px solid #DCDCDC;
        border-radius: 5px;
        outline: 0px;
    }

    /* --- 下拉列表中的项目 --- */
    QComboBox QAbstractItemView::item {
        color: #212121;
        background-color: transparent;
        padding: 8px 10px;
    }
    QComboBox QAbstractItemView::item:selected, 
    QComboBox QAbstractItemView::item:hover {
        background-color: #0078D7;
        color: #FFFFFF;
    }

    /* --- 按钮 --- */
    QPushButton {
        background-color: #EAEAEA;
        color: #212121;
        border: 1px solid #DCDCDC;
        border-radius: 5px;
        padding: 8px 16px;
        font-weight: 500;
    }
    QPushButton:hover {
        background-color: #F0F0F0;
        border-color: #C0C0C0;
    }
    QPushButton:pressed {
        background-color: #0078D7;
        color: #FFFFFF;
        border-color: #005A9E;
    }

    /* --- 复选框 --- */
    QCheckBox {
        spacing: 10px;
    }
    QCheckBox::indicator {
        width: 20px;
        height: 20px;
        border-radius: 4px;
        border: 1px solid #DCDCDC;
        background-color: #FFFFFF;
    }
    QCheckBox::indicator:hover {
        border-color: #C0C0C0;
    }
    QCheckBox::indicator:checked {
        background-color: #0078D7;
        border-color: #0078D7;
    }
"""

# --- 配置文件管理器 ---
class ConfigManager:
    def __init__(self):
        config_dir = os.path.join(os.path.expanduser("~"), ".config", "desktophotbar")
        self.config_path = os.path.join(config_dir, "config.json")
        os.makedirs(config_dir, exist_ok=True)
    def save(self, data):
        try:
            with open(self.config_path, 'w', encoding='utf-8') as f:
                json.dump(data, f, indent=4, ensure_ascii=False)
        except Exception as e: print(f"Error saving config: {e}")
    def load(self):
        if not os.path.exists(self.config_path): return {}
        try:
            with open(self.config_path, 'r', encoding='utf-8') as f:
                return json.load(f)
        except (json.JSONDecodeError, Exception) as e:
            print(f"Error loading config: {e}. Using default.")
            return {}

# --- 对话框 ---
class SlotSettingsDialog(QDialog):
    def __init__(self, app_name, icon_path, parent=None):
        super().__init__(parent)
        self.setWindowTitle("物品栏设置")
        self.setStyleSheet(MODERN_LIGHT_STYLE)
        self.layout = QVBoxLayout(self)
        self.layout.addWidget(QLabel("应用名称:"))
        self.name_edit = QLineEdit(self)
        self.name_edit.setText(app_name)
        self.layout.addWidget(self.name_edit)
        self.layout.addWidget(QLabel("图标路径:"))
        self.icon_edit = QLineEdit(self)
        self.icon_edit.setText(icon_path)
        self.layout.addWidget(self.icon_edit)
        self.buttons = QDialogButtonBox(QDialogButtonBox.Ok | QDialogButtonBox.Cancel, self)
        self.buttons.button(QDialogButtonBox.Ok).setIcon(QIcon())
        self.buttons.button(QDialogButtonBox.Cancel).setIcon(QIcon())
        self.buttons.accepted.connect(self.accept)
        self.buttons.rejected.connect(self.reject)
        self.layout.addWidget(self.buttons)
    def get_data(self): return self.name_edit.text(), self.icon_edit.text()

# --- 总设置对话框 ---
class GeneralSettingsDialog(QDialog):
    settingsChanged = pyqtSignal(dict)
    SCALE_PRESETS = [0.25, 0.5, 0.75, 1.0, 1.25, 1.5, 2.0, 2.5, 3.0, 3.5, 4.0, 4.5, 5.0, 7.5, 10.0]
    def __init__(self, settings, parent=None):
        super().__init__(parent)
        self.setWindowTitle("总设置")
        self.setMinimumWidth(300)
        self.setStyleSheet(MODERN_LIGHT_STYLE)
        self.layout = QVBoxLayout(self)
        self.layout.addWidget(QLabel("窗口层级:"))
        self.level_combo = QComboBox(self)
        self.level_combo.addItems(["总在最前", "正常", "置于底层 (桌面部件)"])
        self.level_combo.setCurrentIndex(settings.get('level', 0))
        self.level_combo.currentIndexChanged.connect(self.on_settings_changed)
        self.layout.addWidget(self.level_combo)
        self.lock_checkbox = QCheckBox("锁定窗口位置 (不可拖动)", self)
        self.lock_checkbox.setChecked(not settings.get('is_movable', True))
        self.lock_checkbox.stateChanged.connect(self.on_settings_changed)
        self.layout.addWidget(self.lock_checkbox)
        self.layout.addWidget(QLabel("缩放比例:"))
        self.scale_combo = QComboBox(self)
        for scale in self.SCALE_PRESETS:
            self.scale_combo.addItem(f"{scale:.2f}x", userData=scale)
        current_scale = settings.get('scale', 3.0)
        closest_preset = min(self.SCALE_PRESETS, key=lambda x: abs(x - current_scale))
        self.scale_combo.setCurrentIndex(self.SCALE_PRESETS.index(closest_preset))
        self.scale_combo.currentIndexChanged.connect(self.on_settings_changed)
        self.layout.addWidget(self.scale_combo)
        self.close_button = QDialogButtonBox(QDialogButtonBox.Close, self)
        self.close_button.button(QDialogButtonBox.Close).setIcon(QIcon())
        self.close_button.rejected.connect(self.reject)
        self.layout.addWidget(self.close_button)
    def on_settings_changed(self):
        settings = {
            'level': self.level_combo.currentIndex(),
            'is_movable': not self.lock_checkbox.isChecked(),
            'scale': self.scale_combo.currentData()
        }
        self.settingsChanged.emit(settings)

# --- 主窗口 ---
class HotbarWindow(QMainWindow):
    def __init__(self):
        super().__init__()
        self.config_manager = ConfigManager()
        self.settings = {}
        self.slots = [None] * 9
        self.load_config()
        self.BASE_WIDTH, self.BASE_HEIGHT = 182, 22
        self.general_settings_dialog = None
        self.current_hover_slot = -1
        self.SLOT_GEOMETRIES = [(3,3,16,16),(23,3,16,16),(43,3,16,16),(63,3,16,16),(83,3,16,16),(103,3,16,16),(123,3,16,16),(143,3,16,16),(163,3,16,16)]
        self.SELECTION_GEOMETRIES = [(-1,-1,24,23),(19,-1,24,23),(39,-1,24,23),(59,-1,24,23),(79,-1,24,23),(99,-1,24,23),(119,-1,24,23),(139,-1,24,23),(159,-1,24,23)]
        self.initUI()

    def initUI(self):
        self.setWindowTitle('HotBar')
        self.setAttribute(Qt.WA_TranslucentBackground)
        self.setMouseTracking(True)
        central_widget = QWidget(self)
        central_widget.setMouseTracking(True)
        self.setCentralWidget(central_widget)
        self.background_label = QLabel(self.centralWidget())
        self.background_label.setMouseTracking(True)
        self.background_label.setAcceptDrops(True)
        self.original_pixmap = self.loadPixmap(self.findImagePath('hotbar.png'))
        self.selection_pixmap = self.loadPixmap(self.findImagePath('hotbar_selection.png'))
        self.selection_label = QLabel(self.background_label)
        self.selection_label.setAttribute(Qt.WA_TransparentForMouseEvents)
        self.selection_label.hide()
        self.slot_labels = []
        for i in range(9):
            slot_label = QLabel(self.background_label)
            slot_label.setAlignment(Qt.AlignCenter)
            slot_label.setAcceptDrops(True)
            slot_label.setMouseTracking(True)
            slot_label.setContextMenuPolicy(Qt.CustomContextMenu)
            slot_label.customContextMenuRequested.connect(lambda point, index=i: self.showContextMenu(point, index))
            slot_label.mousePressEvent = lambda event, index=i: self.slotClicked(event, index)
            slot_label.dragEnterEvent = self.dragEnterEvent
            slot_label.dropEvent = lambda event, index=i: self.dropEvent(event, index)
            self.slot_labels.append(slot_label)
        
        self.applyGeneralSettings(self.settings, is_init=True)
        if 'window_position' not in self.settings: self.centerWindow()

    def updateLayout(self):
        scale = self.settings.get('scale', 3.0)
        width = int(self.BASE_WIDTH * scale)
        height = int(self.BASE_HEIGHT * scale)
        self.setFixedSize(width, height)
        self.centralWidget().setFixedSize(width, height)
        self.background_label.setFixedSize(width, height)
        if self.original_pixmap:
            self.background_label.setPixmap(self.original_pixmap.scaled(width, height, Qt.IgnoreAspectRatio, Qt.FastTransformation))
        for i, label in enumerate(self.slot_labels):
            base_geom = self.SLOT_GEOMETRIES[i]
            x, y, w, h = [int(val * scale) for val in base_geom]
            label.setGeometry(x, y, w, h)
        if self.selection_pixmap:
            base_geom = self.SELECTION_GEOMETRIES[0]
            w, h = int(base_geom[2] * scale), int(base_geom[3] * scale)
            self.selection_label.setPixmap(self.selection_pixmap.scaled(w, h, Qt.IgnoreAspectRatio, Qt.FastTransformation))
            self.selection_label.setFixedSize(w, h)
        for i in range(9): self.updateSlotDisplay(i)

    def mousePressEvent(self, event):
        if event.button() == Qt.LeftButton and self.settings.get('is_movable', True):
            self.drag_position = event.globalPos() - self.frameGeometry().topLeft()
            event.accept()
            
    def mouseMoveEvent(self, event):
        if event.buttons() == Qt.LeftButton and hasattr(self, 'drag_position') and self.settings.get('is_movable', True):
            self.move(event.globalPos() - self.drag_position)
            event.accept()
        else:
            pos = self.background_label.mapFromGlobal(event.globalPos())
            scale = self.settings.get('scale', 3.0)
            hover_found = False
            for i in range(9):
                base_geom = self.SELECTION_GEOMETRIES[i]
                x, y, w, h = [int(val * scale) for val in base_geom]
                if x <= pos.x() < x + w and y <= pos.y() < y + h:
                    if self.current_hover_slot != i:
                        self.selection_label.move(x, y)
                        self.selection_label.show()
                        self.selection_label.raise_()
                        self.current_hover_slot = i
                    hover_found = True
                    break
            if not hover_found and self.current_hover_slot != -1:
                self.selection_label.hide()
                self.current_hover_slot = -1
        super().mouseMoveEvent(event)

    def mouseReleaseEvent(self, event):
        if hasattr(self, 'drag_position'):
            delattr(self, 'drag_position')
            self.save_config()
        super().mouseReleaseEvent(event)

    def closeEvent(self, event):
        self.save_config()
        super().closeEvent(event)

    def save_config(self):
        pos = self.pos()
        self.settings['window_position'] = {'x': pos.x(), 'y': pos.y()}
        config_data = {'settings': self.settings, 'slots': self.slots}
        self.config_manager.save(config_data)

    def load_config(self):
        config_data = self.config_manager.load()
        self.settings = {'level': 0, 'is_movable': True, 'scale': 3.0}
        self.settings.update(config_data.get('settings', {}))
        self.slots = config_data.get('slots', [None] * 9)
        pos_data = self.settings.get('window_position')
        if pos_data: self.move(pos_data.get('x', 0), pos_data.get('y', 0))

    def removeFromHotbar(self, slot_index):
        self.slots[slot_index] = None
        self.updateSlotDisplay(slot_index)
        self.save_config()

    def processDesktopFile(self, file_path, slot_index):
        try:
            with open(file_path, 'r', encoding='utf-8') as f: content = f.read()
            app_info = self.parseDesktopFile(content)
            if app_info and app_info.get('exec'):
                app_info['path'] = file_path
                self.slots[slot_index] = app_info
                self.updateSlotDisplay(slot_index)
                self.save_config()
        except Exception as e: QMessageBox.critical(self, "错误", f"读取文件失败: {str(e)}")

    def openSlotSettings(self, slot_index):
        app_info = self.slots[slot_index]
        if not app_info: return
        dialog = SlotSettingsDialog(app_info.get('name', ''), app_info.get('icon', ''), self)
        if dialog.exec_() == QDialog.Accepted:
            new_name, new_icon = dialog.get_data()
            self.slots[slot_index]['name'] = new_name
            self.slots[slot_index]['icon'] = new_icon
            self.updateSlotDisplay(slot_index)
            self.save_config()

    def openGeneralSettings(self):
        if self.general_settings_dialog is None:
            self.general_settings_dialog = GeneralSettingsDialog(self.settings, self)
            self.general_settings_dialog.settingsChanged.connect(self.applyGeneralSettings)
            self.general_settings_dialog.finished.connect(self.on_general_settings_closed)
            self.general_settings_dialog.show()
        else:
            self.general_settings_dialog.activateWindow()
            self.general_settings_dialog.raise_()

    def on_general_settings_closed(self):
        self.general_settings_dialog = None
        self.save_config()

    def applyGeneralSettings(self, new_settings, is_init=False):
        self.settings.update(new_settings)
        level = self.settings.get('level', 0)
        flags = Qt.FramelessWindowHint
        if level == 0: flags |= Qt.WindowStaysOnTopHint
        elif level == 2: flags |= Qt.WindowStaysOnBottomHint
        if self.windowFlags() != flags:
            self.setWindowFlags(flags)
            if not is_init: self.show()
        self.updateLayout()

    def leaveEvent(self, event):
        self.selection_label.hide()
        self.current_hover_slot = -1
        super().leaveEvent(event)
        
    def showContextMenu(self, point, slot_index):
        app_info = self.slots[slot_index]
        context_menu = QMenu(self)
        context_menu.setAttribute(Qt.WA_TranslucentBackground)
        
        general_settings_action = QAction("总设置...", self)
        general_settings_action.triggered.connect(self.openGeneralSettings)
        quit_action = QAction("退出", self)
        quit_action.triggered.connect(QApplication.instance().quit)
        
        if app_info:
            title_action = QAction(app_info.get('name', '未知应用'), self)
            title_action.setEnabled(False)
            launch_action = QAction("启动应用", self)
            launch_action.triggered.connect(lambda: self.launchApp(slot_index))
            remove_action = QAction("从物品栏移除", self)
            remove_action.triggered.connect(lambda: self.removeFromHotbar(slot_index))
            slot_settings_action = QAction("此物品栏设置...", self)
            slot_settings_action.triggered.connect(lambda: self.openSlotSettings(slot_index))
            context_menu.addAction(title_action)
            context_menu.addAction(launch_action)
            context_menu.addAction(remove_action)
            context_menu.addAction(slot_settings_action)
        else:
            title_action = QAction("空白物品栏", self)
            title_action.setEnabled(False)
            add_app_action = QAction("拖放desktop文件", self)
            add_app_action.setEnabled(False)
            context_menu.addAction(title_action)
            context_menu.addAction(add_app_action)
            
        context_menu.addAction(general_settings_action)
        context_menu.addAction(quit_action)
        
        book_image_path = self.findImagePath('book.png')
        if book_image_path:
            book_image_path = book_image_path.replace('\\', '/')
            context_menu.setFixedSize(150, 184)
            
            stylesheet = f"""
                QMenu {{
                    background-color: transparent;
                    background-image: url({book_image_path});
                    background-repeat: no-repeat;
                    background-position: center;
                    border: none;
                }}
                QMenu::item {{
                    color: #402A18;
                    background-color: transparent;
                    padding: 3px 25px;
                    margin: 2px 0px;
                    font-weight: 500;
                }}
                QMenu::item:selected {{
                    color: #000000;
                    font-weight: bold;
                    background-color: rgba(0, 0, 0, 0.05);
                    border-radius: 3px;
                }}
                QMenu::item:disabled {{
                    color: #000000;
                    font-weight: bold;
                    padding-top: 18px;
                    padding-bottom: 5px;
                }}
            """
            context_menu.setStyleSheet(stylesheet)
            
        context_menu.exec_(self.slot_labels[slot_index].mapToGlobal(point))
        
    def updateSlotDisplay(self, slot_index):
        slot_label = self.slot_labels[slot_index]
        app_info = self.slots[slot_index]
        if slot_label.width() == 0: return
        icon_size = int(slot_label.width() * 0.8)
        if app_info and app_info.get('icon'):
            icon = self.getBestIcon(app_info['icon'])
            if icon:
                pixmap = icon.pixmap(QSize(icon_size, icon_size))
                if not pixmap.isNull():
                    slot_label.setPixmap(pixmap)
                    slot_label.setText("")
                    return
        slot_label.clear()
        if app_info and app_info.get('name'):
            slot_label.setText(app_info['name'][:2])
            font_size = max(8, int(slot_label.height() * 0.5))
            slot_label.setStyleSheet(f"background-color: transparent; color: white; font-weight: bold; font-size: {font_size}px;")
        else: slot_label.setText("")
        
    def getBestIcon(self, icon_name):
        icon = None
        if icon_name and QIcon.hasThemeIcon(icon_name):
            icon = QIcon.fromTheme(icon_name)
            if not icon.isNull(): return icon
        if icon_name and (icon_name.startswith('/') or icon_name.startswith('.')) and os.path.exists(icon_name):
            icon = QIcon(icon_name)
            if not icon.isNull(): return icon
        pixmap_path = f"/usr/share/pixmaps/{icon_name}.png"
        if os.path.exists(pixmap_path):
            icon = QIcon(pixmap_path)
            if not icon.isNull(): return icon
        return icon if icon and not icon.isNull() else None
        
    def findImagePath(self, image_name):
        if hasattr(sys, '_MEIPASS'):
            base_path = sys._MEIPASS
        else:
            base_path = os.path.abspath(".")
        image_path = os.path.join(base_path, 'assets', image_name)
        if os.path.exists(image_path):
            return image_path
        fallback_path = os.path.join(base_path, image_name)
        if os.path.exists(fallback_path):
            return fallback_path
        print(f"Warning: Image '{image_name}' not found at '{image_path}' or '{fallback_path}'")
        return None

    def loadPixmap(self, path, fallback_func=None):
        if path:
            pixmap = QPixmap(path)
            if not pixmap.isNull(): return pixmap
        if fallback_func: fallback_func()
        return None
        
    def setFallbackBackground(self):
        self.background_label.setStyleSheet("background-color: #2d2d2d; border: 2px solid #555; border-radius: 8px;")
        print("未找到 hotbar.png，使用备用背景")
        
    def centerWindow(self):
        screen_geometry = QApplication.primaryScreen().availableGeometry()
        self.move((screen_geometry.width() - self.width()) // 2, screen_geometry.height() - self.height() - 40)
        
    def slotClicked(self, event, slot_index):
        if event.button() == Qt.LeftButton: self.launchApp(slot_index)
        
    def launchApp(self, slot_index):
        app_info = self.slots[slot_index]
        if app_info and app_info.get('exec'):
            try:
                exec_command = app_info['exec'].split('%')[0].strip()
                subprocess.Popen(['nohup', 'sh', '-c', exec_command, '&'], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, start_new_session=True)
            except Exception as e: QMessageBox.critical(self, "错误", f"启动应用失败: {str(e)}")
            
    def dragEnterEvent(self, event):
        if event.mimeData().hasUrls(): event.acceptProposedAction()
        
    def dropEvent(self, event, slot_index):
        urls = event.mimeData().urls()
        if urls:
            file_path = urls[0].toLocalFile()
            if file_path.endswith('.desktop'): self.processDesktopFile(file_path, slot_index)
            else: QMessageBox.warning(self, "错误", "请拖放 .desktop 文件")
            
    def parseDesktopFile(self, content):
        app_info = {}
        for line in content.split('\n'):
            if '=' in line:
                key, value = line.split('=', 1)
                if key.strip() == 'Name': app_info['name'] = value.strip()
                elif key.strip() == 'Icon': app_info['icon'] = value.strip()
                elif key.strip() == 'Exec': app_info['exec'] = value.strip()
        return app_info

def main():
    # 在启动 QApplication 之前，执行插件注入逻辑
    setup_fcitx5_im_plugin()

    QApplication.setAttribute(Qt.AA_EnableHighDpiScaling, True)
    QApplication.setAttribute(Qt.AA_UseHighDpiPixmaps, True)
    app = QApplication(sys.argv)
    window = HotbarWindow()
    window.show()
    sys.exit(app.exec_())

if __name__ == '__main__':
    main()