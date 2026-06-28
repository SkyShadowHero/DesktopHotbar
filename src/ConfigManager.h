#ifndef CONFIGMANAGER_H
#define CONFIGMANAGER_H

#include <QVariantMap>
#include <QString>

class ConfigManager {
public:
    ConfigManager();
    void save(const QVariantMap &data);
    QVariantMap load();

private:
    QString configPath;
};

#endif // CONFIGMANAGER_H
