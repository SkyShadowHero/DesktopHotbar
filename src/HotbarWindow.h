#ifndef HOTBARWINDOW_H
#define HOTBARWINDOW_H

#include <QMainWindow>
#include <QVariantMap>
#include <QVariantList>
#include <QPoint>
#include <QPixmap>
#include <QVector>
#include "ConfigManager.h"

class QLabel;
class SlotLabel;
class GeneralSettingsDialog;

class HotbarWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit HotbarWindow(QWidget *parent = nullptr);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void closeEvent(QCloseEvent *event) override;
    void leaveEvent(QEvent *event) override;

private:
    void launchApp(int slotIndex);
    void processDesktopFile(const QString &filePath, int slotIndex);
    void showContextMenu(const QPoint &pos, int slotIndex);
    void removeFromHotbar(int slotIndex);
    void openSlotSettings(int slotIndex);
    void openGeneralSettings();
    void onGeneralSettingsClosed();

    void initUI();
    void updateLayout();
    void applyGeneralSettings(const QVariantMap &newSettings, bool isInit = false);
    void saveConfig();
    void loadConfig();
    void updateSlotDisplay(int slotIndex);
    void centerWindow();
    QIcon getBestIcon(const QString &iconName);
    QPixmap loadPixmap(const QString &path);
    QVariantMap parseDesktopFile(const QString &content);

    ConfigManager configManager;
    QVariantMap settings;
    QVariantList slotData;   // 9 items, each QVariantMap or null

    static constexpr int BASE_WIDTH = 182;
    static constexpr int BASE_HEIGHT = 22;
    static constexpr int SLOT_COUNT = 9;

    GeneralSettingsDialog *generalSettingsDialog = nullptr;
    int currentHoverSlot = -1;

    QLabel *backgroundLabel;
    QPixmap originalPixmap;
    QPixmap selectionPixmap;
    QLabel *selectionLabel;
    QVector<SlotLabel *> slotLabels;

    QPoint dragPosition;
    bool dragging = false;
};

#endif // HOTBARWINDOW_H
