#include <QApplication>
#include <QFile>
#include <QIcon>
#include <QDebug>
#include "HotbarWindow.h"

// Fcitx5 输入法：设置环境变量即可 (需系统安装 fcitx5-qt6)
static void setupFcitx5InputMethod() {
#ifdef Q_OS_LINUX
    if (qEnvironmentVariableIsEmpty("QT_IM_MODULE")) {
        qputenv("QT_IM_MODULE", "fcitx");
    }
#endif
}

int main(int argc, char *argv[]) {
    setupFcitx5InputMethod();

    // Wayland: 确保 app_id 在协议层正确传递 (KWin 靠它识别窗口并记忆位置)
    if (qEnvironmentVariableIsEmpty("QT_WAYLAND_APP_ID")) {
        qputenv("QT_WAYLAND_APP_ID", "DesktopHotbar");
    }

    QApplication app(argc, argv);
    app.setApplicationName("DesktopHotbar");
    app.setApplicationVersion("0.0.0.1");

    // 设置应用图标
    app.setWindowIcon(QIcon(":/assets/icon.png"));

    // Wayland: 设置 app_id — KWin 靠它识别窗口并记住位置
    app.setDesktopFileName("DesktopHotbar");

    qInfo() << "运行平台:" << QGuiApplication::platformName();

    HotbarWindow window;
    window.show();

    return app.exec();
}
