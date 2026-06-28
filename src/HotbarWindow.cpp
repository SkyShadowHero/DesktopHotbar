#include "HotbarWindow.h"
#include "SlotLabel.h"
#include "SlotSettingsDialog.h"
#include "GeneralSettingsDialog.h"

#include <QApplication>
#include <QScreen>
#include <QVBoxLayout>
#include <QMenu>
#include <QAction>
#include <QWidgetAction>
#include <QMouseEvent>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QProcess>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QMessageBox>
#include <QWindow>
#include <QGuiApplication>
#include <QDebug>

// --- 硬编码的格子几何数据 (Minecraft hotbar 素材) ---
static const int SLOT_GEOMETRIES[9][4] = {
    {3,3,16,16}, {23,3,16,16}, {43,3,16,16},
    {63,3,16,16}, {83,3,16,16}, {103,3,16,16},
    {123,3,16,16}, {143,3,16,16}, {163,3,16,16}
};

static const int SELECTION_GEOMETRIES[9][4] = {
    {-1,-1,24,23}, {19,-1,24,23}, {39,-1,24,23},
    {59,-1,24,23}, {79,-1,24,23}, {99,-1,24,23},
    {119,-1,24,23}, {139,-1,24,23}, {159,-1,24,23}
};

HotbarWindow::HotbarWindow(QWidget *parent)
    : QMainWindow(parent)
{
    // 无边框窗口 (必须在 show 前设置)
    setWindowFlags(Qt::FramelessWindowHint | Qt::Window);
    setAttribute(Qt::WA_TranslucentBackground);
    setWindowTitle("HotBar");

    // 默认设置
    settings["is_movable"] = true;
    settings["scale"] = 3.0;
    slotData = QVariantList(SLOT_COUNT);  // 9 个空 QVariant

    loadConfig();
    initUI();
}

void HotbarWindow::initUI() {
    setMouseTracking(true);

    // 中央部件
    QWidget *central = new QWidget(this);
    central->setMouseTracking(true);
    setCentralWidget(central);

    // 背景标签 (hotbar.png)
    backgroundLabel = new QLabel(central);
    backgroundLabel->setMouseTracking(true);
    backgroundLabel->setAcceptDrops(true);

    originalPixmap = loadPixmap(":/assets/hotbar.png");
    if (originalPixmap.isNull()) {
        backgroundLabel->setStyleSheet(
            "background-color: #2d2d2d; border: 2px solid #555; border-radius: 8px;");
        qWarning() << "未找到 hotbar.png，使用备用背景";
    }

    // 选中高亮 (hotbar_selection.png)
    selectionPixmap = loadPixmap(":/assets/hotbar_selection.png");
    selectionLabel = new QLabel(backgroundLabel);
    selectionLabel->setAttribute(Qt::WA_TransparentForMouseEvents);
    selectionLabel->hide();

    // 创建 9 个格子
    for (int i = 0; i < SLOT_COUNT; ++i) {
        auto *slot = new SlotLabel(i, backgroundLabel);
        connect(slot, &SlotLabel::clicked, this, &HotbarWindow::launchApp);
        connect(slot, &SlotLabel::fileDropped, this, &HotbarWindow::processDesktopFile);
        connect(slot, &QWidget::customContextMenuRequested, this,
                [this, i](const QPoint &pos) { showContextMenu(pos, i); });
        slotLabels.append(slot);
    }

    applyGeneralSettings(settings, true);

    // Wayland: 位置由 compositor 全权决定, 客户端不应干预
    // X11: 恢复上次保存的窗口位置
    if (QGuiApplication::platformName() != "wayland") {
        QVariantMap posData = settings.value("window_position").toMap();
        if (!posData.isEmpty()) {
            move(posData["x"].toInt(), posData["y"].toInt());
        } else {
            centerWindow();
        }
    }
}

void HotbarWindow::updateLayout() {
    double scale = settings.value("scale", 3.0).toDouble();
    int w = static_cast<int>(BASE_WIDTH * scale);
    int h = static_cast<int>(BASE_HEIGHT * scale);

    setFixedSize(w, h);
    centralWidget()->setFixedSize(w, h);
    backgroundLabel->setFixedSize(w, h);

    // 背景图缩放
    if (!originalPixmap.isNull()) {
        backgroundLabel->setPixmap(
            originalPixmap.scaled(w, h, Qt::IgnoreAspectRatio, Qt::FastTransformation));
    }

    // 每个格子位置缩放
    for (int i = 0; i < SLOT_COUNT; ++i) {
        const int *geom = SLOT_GEOMETRIES[i];
        int x = static_cast<int>(geom[0] * scale);
        int y = static_cast<int>(geom[1] * scale);
        int sw = static_cast<int>(geom[2] * scale);
        int sh = static_cast<int>(geom[3] * scale);
        slotLabels[i]->setGeometry(x, y, sw, sh);
    }

    // 选中高亮图缩放
    if (!selectionPixmap.isNull()) {
        const int *selGeom = SELECTION_GEOMETRIES[0];
        int sw = static_cast<int>(selGeom[2] * scale);
        int sh = static_cast<int>(selGeom[3] * scale);
        selectionLabel->setPixmap(
            selectionPixmap.scaled(sw, sh, Qt::IgnoreAspectRatio, Qt::FastTransformation));
        selectionLabel->setFixedSize(sw, sh);
    }

    // 刷新所有格子显示
    for (int i = 0; i < SLOT_COUNT; ++i) {
        updateSlotDisplay(i);
    }
}

// --- 鼠标事件：拖拽窗口 / 悬停高亮 ---

void HotbarWindow::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton && settings.value("is_movable", true).toBool()) {
        if (QGuiApplication::platformName() == "wayland") {
            if (auto *win = windowHandle()) {
                win->startSystemMove();
            }
        } else {
            dragPosition = event->globalPosition().toPoint() - frameGeometry().topLeft();
            dragging = true;
        }
        event->accept();
    }
}

void HotbarWindow::mouseMoveEvent(QMouseEvent *event) {
    if (dragging && (event->buttons() & Qt::LeftButton)
        && settings.value("is_movable", true).toBool()) {
        // X11: 手动移动窗口（Wayland 下由 startSystemMove 接管，不会进入此分支）
        move(event->globalPosition().toPoint() - dragPosition);
        event->accept();
    } else {
        // 悬停高亮检测
        QPoint pos = backgroundLabel->mapFromGlobal(event->globalPosition().toPoint());
        double scale = settings.value("scale", 3.0).toDouble();
        bool hoverFound = false;

        for (int i = 0; i < SLOT_COUNT; ++i) {
            const int *geom = SELECTION_GEOMETRIES[i];
            int x = static_cast<int>(geom[0] * scale);
            int y = static_cast<int>(geom[1] * scale);
            int w = static_cast<int>(geom[2] * scale);
            int h = static_cast<int>(geom[3] * scale);
            if (pos.x() >= x && pos.x() < x + w && pos.y() >= y && pos.y() < y + h) {
                if (currentHoverSlot != i) {
                    selectionLabel->move(x, y);
                    selectionLabel->show();
                    selectionLabel->raise();
                    currentHoverSlot = i;
                }
                hoverFound = true;
                break;
            }
        }

        if (!hoverFound && currentHoverSlot != -1) {
            selectionLabel->hide();
            currentHoverSlot = -1;
        }
    }
    QMainWindow::mouseMoveEvent(event);
}

void HotbarWindow::mouseReleaseEvent(QMouseEvent *event) {
    if (dragging) {
        dragging = false;
        saveConfig();
    }
    QMainWindow::mouseReleaseEvent(event);
}

void HotbarWindow::closeEvent(QCloseEvent *event) {
    saveConfig();
    QMainWindow::closeEvent(event);
}

void HotbarWindow::leaveEvent(QEvent *event) {
    selectionLabel->hide();
    currentHoverSlot = -1;
    QMainWindow::leaveEvent(event);
}

// --- 配置持久化 ---

void HotbarWindow::saveConfig() {
    // 始终保存窗口位置 (Wayland 下拖动后 pos() 值有效, 恢复留给 compositor)
    QPoint pos = this->pos();
    QVariantMap posMap;
    posMap["x"] = pos.x();
    posMap["y"] = pos.y();
    settings["window_position"] = posMap;

    QVariantMap data;
    data["settings"] = settings;
    data["slots"] = slotData;
    configManager.save(data);
}

void HotbarWindow::loadConfig() {
    QVariantMap data = configManager.load();

    // 合并设置（保留默认值）
    QVariantMap loadedSettings = data.value("settings").toMap();
    for (auto it = loadedSettings.begin(); it != loadedSettings.end(); ++it) {
        settings[it.key()] = it.value();
    }

    // 加载格子数据
    QVariantList loadedSlots = data.value("slots").toList();
    if (loadedSlots.size() == SLOT_COUNT) {
        slotData = loadedSlots;
    }

    // X11: 恢复窗口位置。Wayland: compositor 自动管理，跳过
    if (QGuiApplication::platformName() != "wayland") {
        QVariantMap posData = settings.value("window_position").toMap();
        if (!posData.isEmpty()) {
            move(posData.value("x", 0).toInt(), posData.value("y", 0).toInt());
        }
    }
}

// --- 窗口设置 ---

void HotbarWindow::applyGeneralSettings(const QVariantMap &newSettings, bool isInit) {
    double oldScale = settings.value("scale", 3.0).toDouble();

    // 合并新设置
    for (auto it = newSettings.begin(); it != newSettings.end(); ++it) {
        settings[it.key()] = it.value();
    }

    double newScale = settings.value("scale", 3.0).toDouble();

    // 只在缩放改变时重建布局 (Wayland 下避免不必要的 setFixedSize)
    if (isInit || !qFuzzyCompare(oldScale, newScale)) {
        updateLayout();
    }
}

void HotbarWindow::centerWindow() {
    QRect screenGeometry = QApplication::primaryScreen()->availableGeometry();
    move((screenGeometry.width() - width()) / 2,
         screenGeometry.height() - height() - 40);
}

// --- 右键菜单 ---

void HotbarWindow::showContextMenu(const QPoint &pos, int slotIndex) {
    QVariantMap appInfo = slotData[slotIndex].toMap();
    auto *contextMenu = new QMenu(this);
    contextMenu->setAttribute(Qt::WA_TranslucentBackground);
    contextMenu->setWindowFlag(Qt::FramelessWindowHint, true);
    contextMenu->setWindowFlag(Qt::NoDropShadowWindowHint, true);

    auto *generalAction = new QAction("总设置...", contextMenu);
    connect(generalAction, &QAction::triggered, this, &HotbarWindow::openGeneralSettings);

    if (!appInfo.isEmpty()) {
        QString title = appInfo.value("name", "未知应用").toString();
        // 长名称拆成两排 (优先在空格处断行)
        if (title.length() > 14) {
            if (title.length() > 24) title = title.left(22) + "..";
            int mid = title.length() / 2;
            int spaceIdx = title.lastIndexOf(' ', mid);
            if (spaceIdx > 4 && spaceIdx < title.length() - 4) {
                title = title.left(spaceIdx) + "\n" + title.mid(spaceIdx + 1);
            } else {
                title = title.left(mid) + "\n" + title.mid(mid);
            }
        }
        auto *titleLabel = new QLabel(title);
        titleLabel->setAlignment(Qt::AlignCenter);
        titleLabel->setWordWrap(true);
        titleLabel->setStyleSheet("color: #000000; font-weight: bold; padding: 14px 16px 4px 16px; background: transparent;");
        auto *titleAction = new QWidgetAction(contextMenu);
        titleAction->setDefaultWidget(titleLabel);
        auto *launchAction = new QAction("启动应用", contextMenu);
        connect(launchAction, &QAction::triggered, this, [this, slotIndex]() { launchApp(slotIndex); });
        auto *removeAction = new QAction("从物品栏移除", contextMenu);
        connect(removeAction, &QAction::triggered, this, [this, slotIndex]() { removeFromHotbar(slotIndex); });
        auto *slotSettingsAction = new QAction("此物品栏设置...", contextMenu);
        connect(slotSettingsAction, &QAction::triggered, this, [this, slotIndex]() { openSlotSettings(slotIndex); });

        contextMenu->addAction(titleAction);
        contextMenu->addAction(launchAction);
        contextMenu->addAction(removeAction);
        contextMenu->addAction(slotSettingsAction);
        contextMenu->addAction(generalAction);
    } else {
        auto *emptyLabel = new QLabel("空白物品栏");
        emptyLabel->setAlignment(Qt::AlignCenter);
        emptyLabel->setStyleSheet("color: #000000; font-weight: bold; padding: 14px 16px 4px 16px; background: transparent;");
        auto *emptyAction = new QWidgetAction(contextMenu);
        emptyAction->setDefaultWidget(emptyLabel);

        auto *hintLabel = new QLabel("拖放desktop文件");
        hintLabel->setAlignment(Qt::AlignCenter);
        hintLabel->setStyleSheet("color: #000000; font-weight: bold; padding: 2px 16px 4px 16px; background: transparent;");
        auto *hintAction = new QWidgetAction(contextMenu);
        hintAction->setDefaultWidget(hintLabel);

        auto *quitAction = new QAction("退出", contextMenu);
        connect(quitAction, &QAction::triggered, qApp, &QApplication::quit);

        contextMenu->addAction(emptyAction);
        contextMenu->addAction(hintAction);
        contextMenu->addAction(generalAction);
        contextMenu->addAction(quitAction);
    }

    contextMenu->setFixedSize(150, 184);
    contextMenu->setStyleSheet(R"(
        QMenu {
            background-color: transparent;
            background-image: url(:/assets/book.svg);
            background-repeat: no-repeat;
            background-position: center;
            border: none;
            outline: none;
        }
        QMenu::item {
            color: #402A18;
            background-color: transparent;
            padding: 4px 16px;
            margin: 1px 0px;
            font-weight: 500;
        }
        QMenu::item:selected {
            color: #000000;
            font-weight: bold;
            background-color: rgba(0,0,0,0.05);
            border-radius: 3px;
        }
    )");

    contextMenu->exec(slotLabels[slotIndex]->mapToGlobal(pos));
    contextMenu->deleteLater();
}

// --- 格子操作 ---

void HotbarWindow::removeFromHotbar(int slotIndex) {
    slotData[slotIndex] = QVariant();
    updateSlotDisplay(slotIndex);
    saveConfig();
}

void HotbarWindow::processDesktopFile(const QString &filePath, int slotIndex) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "错误",
                              QString("读取文件失败: %1").arg(file.errorString()));
        return;
    }
    QString content = QString::fromUtf8(file.readAll());
    QVariantMap appInfo = parseDesktopFile(content);
    if (appInfo.isEmpty() || !appInfo.contains("exec")) {
        QMessageBox::warning(this, "错误", "无法解析 .desktop 文件或缺少 Exec 字段");
        return;
    }
    appInfo["path"] = filePath;
    slotData[slotIndex] = appInfo;
    updateSlotDisplay(slotIndex);
    saveConfig();
}

void HotbarWindow::openSlotSettings(int slotIndex) {
    QVariantMap appInfo = slotData[slotIndex].toMap();
    if (appInfo.isEmpty()) return;

    auto *dialog = new SlotSettingsDialog(
        appInfo.value("name").toString(),
        appInfo.value("icon").toString(),
        appInfo.value("exec").toString(),
        this);
    if (dialog->exec() == QDialog::Accepted) {
        appInfo["name"] = dialog->appName();
        appInfo["icon"] = dialog->iconPath();
        appInfo["exec"] = dialog->execCommand();
        slotData[slotIndex] = appInfo;
        updateSlotDisplay(slotIndex);
        saveConfig();
    }
    dialog->deleteLater();
}

void HotbarWindow::openGeneralSettings() {
    if (!generalSettingsDialog) {
        generalSettingsDialog = new GeneralSettingsDialog(settings, this);
        connect(generalSettingsDialog, &GeneralSettingsDialog::settingsChanged,
                this, [this](const QVariantMap &s) { applyGeneralSettings(s); });
        connect(generalSettingsDialog, &QDialog::finished,
                this, &HotbarWindow::onGeneralSettingsClosed);
        generalSettingsDialog->show();
    } else {
        generalSettingsDialog->activateWindow();
        generalSettingsDialog->raise();
    }
}

void HotbarWindow::onGeneralSettingsClosed() {
    generalSettingsDialog = nullptr;
    saveConfig();
}

// --- 应用启动 ---

void HotbarWindow::launchApp(int slotIndex) {
    QVariantMap appInfo = slotData[slotIndex].toMap();
    if (appInfo.isEmpty()) return;

    // 优先: 通过 gio launch 启动 .desktop 文件 (行为与桌面环境完全一致)
    QString desktopPath = appInfo.value("path").toString();
    if (!desktopPath.isEmpty() && QFile::exists(desktopPath)) {
        qint64 pid = 0;
        if (QProcess::startDetached("gio", {"launch", desktopPath}, {}, &pid)) {
            return;
        }
    }

    // 回退: 直接执行 Exec 字段 (gio 不可用或 desktop 文件丢失)
    if (!appInfo.contains("exec")) return;

    QString execCommand = appInfo["exec"].toString();
    int pctIdx = execCommand.indexOf('%');
    if (pctIdx >= 0) {
        execCommand = execCommand.left(pctIdx);
    }
    execCommand = execCommand.trimmed();
    if (execCommand.isEmpty()) return;

    QString workingDir = appInfo.value("workdir").toString();
    if (workingDir.isEmpty()) {
        workingDir = QDir::homePath();
    } else if (workingDir.startsWith("~/")) {
        workingDir = QDir::homePath() + workingDir.mid(1);
    }

    // 尝试直接执行 (避免 sh -c 导致进程上下文差异)
    QStringList parts = QProcess::splitCommand(execCommand);
    if (!parts.isEmpty()) {
        QString program = parts.takeFirst();
        QProcess::startDetached(program, parts, workingDir);
    }
}

// --- 图标与显示 ---

void HotbarWindow::updateSlotDisplay(int slotIndex) {
    SlotLabel *slotLabel = slotLabels[slotIndex];
    QVariantMap appInfo = slotData[slotIndex].toMap();

    if (slotLabel->width() == 0) return;

    int iconSize = static_cast<int>(slotLabel->width() * 0.8);

    if (!appInfo.isEmpty() && appInfo.contains("icon")) {
        QIcon icon = getBestIcon(appInfo["icon"].toString());
        if (!icon.isNull()) {
            QPixmap pm = icon.pixmap(QSize(iconSize, iconSize));
            if (!pm.isNull()) {
                slotLabel->setPixmap(pm);
                slotLabel->setText("");
                return;
            }
        }
    }

    // 回退：显示文字
    slotLabel->clear();
    if (!appInfo.isEmpty() && appInfo.contains("name")) {
        QString name = appInfo["name"].toString();
        slotLabel->setText(name.left(2));
        int fontSize = qMax(8, static_cast<int>(slotLabel->height() * 0.5));
        slotLabel->setStyleSheet(QString(
            "background-color: transparent; color: white; font-weight: bold; font-size: %1px;"
        ).arg(fontSize));
    } else {
        slotLabel->setText("");
        slotLabel->setStyleSheet("");
    }
}

QIcon HotbarWindow::getBestIcon(const QString &iconName) {
    if (iconName.isEmpty()) return {};

    // 1. 主题图标
    if (QIcon::hasThemeIcon(iconName)) {
        QIcon icon = QIcon::fromTheme(iconName);
        if (!icon.isNull()) return icon;
    }

    // 2. 绝对/相对路径
    if ((iconName.startsWith('/') || iconName.startsWith('.'))
        && QFile::exists(iconName)) {
        QIcon icon(iconName);
        if (!icon.isNull()) return icon;
    }

    // 3. /usr/share/pixmaps/
    QString pixmapPath = "/usr/share/pixmaps/" + iconName + ".png";
    if (QFile::exists(pixmapPath)) {
        QIcon icon(pixmapPath);
        if (!icon.isNull()) return icon;
    }

    return {};
}

QPixmap HotbarWindow::loadPixmap(const QString &path) {
    QPixmap pm(path);
    return pm;  // 可能为 null
}

QVariantMap HotbarWindow::parseDesktopFile(const QString &content) {
    QVariantMap appInfo;
    const QStringList lines = content.split('\n');
    bool inDesktopEntry = false;

    for (const QString &line : lines) {
        QString trimmed = line.trimmed();

        // 跳过开头注释
        if (!inDesktopEntry && trimmed.startsWith("#!")) continue;

        // 找到 [Desktop Entry] 段开始解析; 遇到其他 [*] 段则停止
        if (trimmed.startsWith('[')) {
            if (trimmed == "[Desktop Entry]") {
                inDesktopEntry = true;
            } else if (inDesktopEntry) {
                break;
            }
            continue;
        }
        if (!inDesktopEntry) continue;

        int idx = line.indexOf('=');
        if (idx > 0) {
            QString key = line.left(idx).trimmed();
            QString value = line.mid(idx + 1).trimmed();
            if (key == "Name") appInfo["name"] = value;
            else if (key == "Icon") appInfo["icon"] = value;
            else if (key == "Exec") appInfo["exec"] = value;
            else if (key == "Path") appInfo["workdir"] = value;
        }
    }
    return appInfo;
}
