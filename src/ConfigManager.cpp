#include "ConfigManager.h"
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStandardPaths>
#include <QDebug>

ConfigManager::ConfigManager() {
    QString configDir = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation)
                        + "/desktophotbar";
    QDir().mkpath(configDir);
    configPath = configDir + "/config.json";
}

void ConfigManager::save(const QVariantMap &data) {
    QFile file(configPath);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "Error saving config:" << file.errorString();
        return;
    }
    QJsonDocument doc(QJsonObject::fromVariantMap(data));
    file.write(doc.toJson(QJsonDocument::Indented));
}

QVariantMap ConfigManager::load() {
    QFile file(configPath);
    if (!file.exists()) return {};
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Error loading config:" << file.errorString();
        return {};
    }
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &error);
    if (error.error != QJsonParseError::NoError) {
        qWarning() << "Error parsing config JSON:" << error.errorString();
        return {};
    }
    return doc.object().toVariantMap();
}
