#ifndef GENERALSETTINGSDIALOG_H
#define GENERALSETTINGSDIALOG_H

#include <QDialog>
#include <QVariantMap>
#include <QComboBox>

class QCheckBox;

class GeneralSettingsDialog : public QDialog {
    Q_OBJECT
public:
    explicit GeneralSettingsDialog(const QVariantMap &settings, QWidget *parent = nullptr);

signals:
    void settingsChanged(const QVariantMap &settings);

private slots:
    void onSettingsChanged();

private:
    QCheckBox *lockCheckbox;
    QCheckBox *smoothHoverCheckbox;
    QComboBox *scaleCombo;
};

#endif // GENERALSETTINGSDIALOG_H
