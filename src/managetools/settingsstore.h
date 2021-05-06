#ifndef SETTINGSSTORE_H
#define SETTINGSSTORE_H

#include <QSettings>
#include <QString>

#include "abstractstore.h"
#include "simplecrypt.h"

class SettingsStore: public AbstractStore
{
    Q_OBJECT

public:

    explicit SettingsStore(const QString &encryptionKey, const QString &fileName, QObject *parent=0);
    explicit SettingsStore(QSettings *settings, const QString &encryptionKey, QObject *parent = 0);

    ~SettingsStore();


    QString groupKey() const;
    void setGroupKey(const QString &groupKey);

    QString value(const QString &key, const QString &defaultValue = QString());
    void setValue(const QString &key, const QString &value);

signals:
    // Property change signals
    void groupKeyChanged();

protected:
    QSettings           *settings_;
    QString             groupKey_;
    SimpleCrypt         crypt_;

};

#endif // SETTINGSSTORE_H
