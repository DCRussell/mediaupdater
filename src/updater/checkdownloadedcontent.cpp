#include "checkdownloadedcontent.h"
#include "stationmanager.h"
#include "logger.h"

#include <QScopedPointer>


CheckDownloadedContent::CheckDownloadedContent(QString boxId, QObject *parent) : QObject(parent)
{
    mSettings.reset( new QSettings(StationManager::Instance().mediaSync() + "/" + boxId +"/checkstorage", QSettings::IniFormat, this) );
    mRevSettings.reset(new QSettings(StationManager::Instance().mediaSync() + "/" + boxId +"/command/revstorage", QSettings::IniFormat, this) );
}

bool CheckDownloadedContent::checkFile(const SContentInfo &info)
{
    mSettings->beginGroup(info.dirName());
    bool ok = mSettings->value(info.fileName(), false).toBool();
    mSettings->endGroup();

    if (!ok && !info.content()->isLocal()) {
        QScopedPointer<QFile> file( new QFile( info.content()->destinationSlash() + info.dirNameSlash()+info.fileName() ) );
        if( file->exists() ) {
            if( !file->remove() ) {
                Logger() << __FILE__ << __LINE__ << "ERROR: Cannot remove file:" << info.content()->destinationSlash() + info.dirNameSlash()+info.fileName();
            } else {
                remove(info);
            }
        } else {
            //Logger() << __FILE__ << __LINE__ << "file" << file->fileName() << "not found";
        }
    }

    return ok;
}

int CheckDownloadedContent::checkRev(const QString &fileName)
{
    return mRevSettings->value( fileName, DEFAULT).toInt();
}

int CheckDownloadedContent::checkRev(const SContentInfo &info)
{
    return checkRev( info.fileName() );
}

void CheckDownloadedContent::setRev(const QString &fileName, CHECK_STATUS st)
{
    mRevSettings->setValue( fileName, st );
}

void CheckDownloadedContent::setRev(const SContentInfo &info, CHECK_STATUS st)
{
    setRev( info.fileName(), st );
}

bool CheckDownloadedContent::checkAllFiles(const QList<SContentInfo> &infoList)
{
    if( infoList.isEmpty() )
        return true;

    bool ok = false;
    for( int i=0; i <infoList.size(); ++i) {
        ok = checkFile( infoList[i]);
        if( !ok )
            break;
    }

    return ok;
}

void CheckDownloadedContent::setTrust(const SContentInfo &info)
{
    mSettings->beginGroup( info.dirName() );
    mSettings->setValue( info.fileName(), true );
    mSettings->endGroup();
}

bool CheckDownloadedContent::trust(const SContentInfo &info)
{
    mSettings->beginGroup( info.dirName() );
    bool trust = mSettings->value(info.fileName(), false).toBool();
    mSettings->endGroup();
    return trust;
}

void CheckDownloadedContent::remove(const SContentInfo &info)
{
    mSettings->beginGroup(info.dirName());
    mSettings->remove(info.fileName());
    mSettings->endGroup();
}
