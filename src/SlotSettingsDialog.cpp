#include "SlotSettingsDialog.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QIcon>

SlotSettingsDialog::SlotSettingsDialog(const QString &appName, const QString &iconPath,
                                       const QString &execCmd,
                                       QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("物品栏设置");
    setMinimumWidth(360);

    auto *layout = new QVBoxLayout(this);

    layout->addWidget(new QLabel("应用名称:"));
    nameEdit = new QLineEdit(this);
    nameEdit->setText(appName);
    layout->addWidget(nameEdit);

    layout->addWidget(new QLabel("图标路径:"));
    iconEdit = new QLineEdit(this);
    iconEdit->setText(iconPath);
    layout->addWidget(iconEdit);

    layout->addWidget(new QLabel("启动命令 (Exec):"));
    execEdit = new QLineEdit(this);
    execEdit->setText(execCmd);
    layout->addWidget(execEdit);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    buttons->button(QDialogButtonBox::Ok)->setIcon(QIcon());
    buttons->button(QDialogButtonBox::Cancel)->setIcon(QIcon());
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addWidget(buttons);
}

QString SlotSettingsDialog::appName() const { return nameEdit->text(); }
QString SlotSettingsDialog::iconPath() const { return iconEdit->text(); }
QString SlotSettingsDialog::execCommand() const { return execEdit->text(); }
