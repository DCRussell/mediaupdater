#include <QCryptographicHash>
#include <QByteArray>
#include <QCoreApplication>

#include "settingsstore.h"

static quint64 getHash(const QString &encryptionKey)
{
    return QCryptographicHash::hash(encryptionKey.toLatin1(), QCryptographicHash::Sha1).toULongLong();
}

SettingsStore::SettingsStore(const QString &encryptionKey, const QString &fileName, QObject *parent):
    AbstractStore(parent), crypt_(getHash(encryptionKey))
{
    QString file = QCoreApplication::applicationDirPath() + "/" + fileName;
    settings_ = new QSettings( file, QSettings::IniFormat, this );
}

SettingsStore::SettingsStore(QSettings *settings, const QString &encryptionKey, QObject *parent):
    AbstractStore(parent), crypt_(getHash(encryptionKey))
{
    settings_ = settings;
    settings_->setParent(this);
}

SettingsStore::~SettingsStore()
{
    delete settings_;
}

QString SettingsStore::groupKey() const
{
    return groupKey_;
}

void SettingsStore::setGroupKey(const QString &groupKey)
{
    if (groupKey_ == groupKey) {
        return;
    }

    groupKey_ = groupKey;
    emit groupKeyChanged();
}

QString SettingsStore::value(const QString &key, const QString &defaultValue)
{
    settings_->sync();
    QString fullKey = groupKey_.isEmpty() ? key : (groupKey_ + '/' + key);
    if (!settings_->contains(fullKey)) {
        return defaultValue;
    }
    return crypt_.decryptToString(settings_->value(fullKey).toString());
}

void SettingsStore::setValue(const QString &key, const QString &val)
{
//    const QString currentValue = value( key, QString::null );
//    if( currentValue == val  )
//        return;

    QString fullKey = groupKey_.isEmpty() ? key : (groupKey_ + '/' + key);
    settings_->setValue(fullKey, crypt_.encryptToString(val));
    settings_->sync();
}
