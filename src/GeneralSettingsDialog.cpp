#include "GeneralSettingsDialog.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QIcon>
#include <QApplication>
#include <QtMath>

static const double SCALE_PRESETS[] = {
    0.25, 0.5, 0.75, 1.0, 1.25, 1.5, 2.0, 2.5, 3.0,
    3.5, 4.0, 4.5, 5.0, 7.5, 10.0
};
static const int SCALE_PRESETS_COUNT = sizeof(SCALE_PRESETS) / sizeof(SCALE_PRESETS[0]);

GeneralSettingsDialog::GeneralSettingsDialog(const QVariantMap &settings, QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("总设置");
    setMinimumWidth(300);

    auto *layout = new QVBoxLayout(this);

    // 锁定窗口位置
    lockCheckbox = new QCheckBox("锁定窗口位置 (不可拖动)", this);
    lockCheckbox->setChecked(!settings.value("is_movable", true).toBool());
    connect(lockCheckbox, &QCheckBox::checkStateChanged,
            this, &GeneralSettingsDialog::onSettingsChanged);
    layout->addWidget(lockCheckbox);

    // 缩放比例
    layout->addWidget(new QLabel("缩放比例:"));
    scaleCombo = new QComboBox(this);
    double currentScale = settings.value("scale", 2.0).toDouble();
    int closestIdx = 0;
    double closestDist = std::abs(SCALE_PRESETS[0] - currentScale);
    for (int i = 0; i < SCALE_PRESETS_COUNT; ++i) {
        QString text = QString::asprintf("%.2fx", SCALE_PRESETS[i]);
        scaleCombo->addItem(text, QVariant(SCALE_PRESETS[i]));
        double dist = std::abs(SCALE_PRESETS[i] - currentScale);
        if (dist < closestDist) { closestDist = dist; closestIdx = i; }
    }
    scaleCombo->setCurrentIndex(closestIdx);
    connect(scaleCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &GeneralSettingsDialog::onSettingsChanged);
    layout->addWidget(scaleCombo);

    // 按钮: 退出程序 + 关闭对话框
    auto *buttons = new QDialogButtonBox(this);
    auto *quitBtn = buttons->addButton("退出程序", QDialogButtonBox::DestructiveRole);
    connect(quitBtn, &QPushButton::clicked, qApp, &QApplication::quit);
    auto *closeBtn = buttons->addButton(QDialogButtonBox::Close);
    closeBtn->setIcon(QIcon());
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addWidget(buttons);
}

void GeneralSettingsDialog::onSettingsChanged() {
    QVariantMap s;
    s["is_movable"] = !lockCheckbox->isChecked();
    s["scale"] = scaleCombo->currentData().toDouble();
    emit settingsChanged(s);
}
