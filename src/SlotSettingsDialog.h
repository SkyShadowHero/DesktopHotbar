#ifndef SLOTSETTINGSDIALOG_H
#define SLOTSETTINGSDIALOG_H

#include <QDialog>

class QLineEdit;

class SlotSettingsDialog : public QDialog {
    Q_OBJECT
public:
    explicit SlotSettingsDialog(const QString &appName, const QString &iconPath,
                                const QString &execCmd,
                                QWidget *parent = nullptr);
    QString appName() const;
    QString iconPath() const;
    QString execCommand() const;

private:
    QLineEdit *nameEdit;
    QLineEdit *iconEdit;
    QLineEdit *execEdit;
};

#endif // SLOTSETTINGSDIALOG_H
