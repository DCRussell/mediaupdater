#include "content.h"
#include "checkdownloadedcontent.h"
#include "logger.h"

#include "stationmanager.h"


#include <QFile>
#include <QDir>
#include <QFileInfo>
#include <QFileInfoList>
#include <QStringList>
#include <QScopedPointer>


Content::Content( CONTENT_TYPE type, QString boxId, bool local ) :
    mContentType(type),
    mLocal(local),
    mBoxId(boxId)
{
    mStringContentType[REVISION]   = "Revision";
    mStringContentType[MUSIC]      = "Music";
    mStringContentType[ADVERT]     = "Advert";
    mStringContentType[REPORT]     = "Report";

    if( REPORT == type ) {
        mSourceDir = StationManager::Instance().reportsDir() +"/" + mBoxId;
        collectRepots();
        return;
    }

    if(mLocal)
        mDest = StationManager::Instance().media() + "/";
    else
        mDest = StationManager::Instance().mediaSync() + "/" + mBoxId + "/";

    if( REVISION == mContentType ) {
        setRevision();
    }
    else if(!mLocal && MUSIC == mContentType)
    {
        collectContentFromFile( mDest + "chanels/chanels" );
        mDest       += StationManager::Instance().musicDir();
    }
    else if ( !mLocal && ADVERT == mContentType )
    {
        collectContentFromFile( mDest + "cron.d/adverttable" );
    }
    else if(mLocal && MUSIC == mContentType )
    {
        mSourceDir  = StationManager::Instance().mediaSync() + "/" + mBoxId + "/" + StationManager::Instance().musicDir();
        mDest       = StationManager::Instance().media() + "/" + StationManager::Instance().musicDir();
        collectContentFromPathWithCheck(mSourceDir);
    }
    else if( mLocal && ADVERT == mContentType )
    {
        mSourceDir  = StationManager::Instance().mediaSync() + "/" + mBoxId + "/" + StationManager::Instance().advertDir();
        mDest       = StationManager::Instance().media() + "/" + StationManager::Instance().advertDir();
        collectContentFromDirWithCheck(mSourceDir);
    }
    else
        Logger() << "Неизвесный тип контента:" << mContentType << mStringContentType[mContentType];
}

Content::~Content()
{
    //
}


void Content::appendInfo( const QString &dirName, const QString &fileName, bool all )
{
    SContentInfo ci;
    ci.setDirName(dirName);
    ci.setFileName(fileName);
    ci.setContent(this);
    if( all ) {
        mStandartSourceFiles.append(ci);
    } else
        mSourceList.append(ci);
}


void Content::appendInfo(const QString &dirName, const QStringList &files, bool all)
{
    for( int i =0; i < files.size(); ++i )
        appendInfo( dirName, files[i], all );
}


CONTENT_TYPE Content::type()
{
    return mContentType;
}


QString Content::sourceDir()
{
    return mSourceDir;
}

QString Content::sourceDirSlash()
{
    if( mSourceDir.isEmpty() )
        return mSourceDir;

    if( mSourceDir.right(1) == "/")
        return mSourceDir;

    return mSourceDir + "/";
}

bool Content::sync()
{
    bool result = false;
    result |= copyToDestination();
    // NOTE: Пока что уберем перезапуск плеера после удаления файлов
    /*result |= */deleteExcessFiles();
    return result;
}

bool Content::isLocal()
{
    return mLocal;
}

bool Content::checkExistFile(const QString &dir, const QString &file)
{
    QString localDir("/%1/");
    if( MUSIC == mContentType ) {
        localDir = localDir.arg(StationManager::Instance().musicDir());
    } else if( ADVERT == mContentType )
        localDir = "/";

    QString filePathLocal   = StationManager::Instance().media() + localDir + dir +"/" + file;
    QFile fileLocal(filePathLocal);

    SContentInfo ci;
    ci.setDirName(dir);
    ci.setFileName(file);
    ci.setContent(this);
    CheckDownloadedContent check(mBoxId);

    return ( check.checkFile(ci) || fileLocal.exists() );
}

bool Content::copyToDestination()
{
    QScopedPointer<QFile> file( new QFile() );
    QString source = sourceDirSlash();
    QString destination = destinationSlash();

    bool result = false;

    for( int i = 0; i < mSourceList.size(); ++i ) {
        if( MUSIC == mContentType ) {
            source      = sourceDirSlash() + mSourceList[i].dirNameSlash();
            destination = destinationSlash() + mSourceList[i].dirNameSlash();
        } else if (REVISION == mContentType ) {
            source      = sourceDirSlash() + mSourceList[i].dirNameSlash();
        }

        file->setFileName( source + mSourceList[i].fileName() );
        QDir dir;
        if( !dir.mkpath( destination ) )
            Logger() << __FILE__ << __LINE__ << "Cannon create path:" << destination;

        QString filePath = destination + mSourceList[i].fileName();
        if( REVISION == mContentType ) {
            QScopedPointer<QFile> curFile(new QFile(filePath));
            QByteArray newData;
            QByteArray oldData;
            CheckDownloadedContent check(mBoxId);
            QFileInfo fi(*file);
            const int type = check.checkRev(fi.fileName());
            if( file->open(QFile::ReadOnly) && (type == CheckDownloadedContent::DOWNLOAD_OK) ) {
                newData = file->readAll();
                file->close();

                if( curFile->open(QFile::ReadOnly ) ) {
                    oldData    = curFile->readAll();
                    curFile->close();
                } else {
                    Logger() << __FILE__ << __LINE__ << "Cannot open current file:" << curFile->fileName();
                }

                if( newData == oldData )
                    continue;
            } else if(type != CheckDownloadedContent::DEFAULT){
                Logger() << __FILE__ << __LINE__ << "Cannot open new file or download false:" << file->fileName();
                continue;
            } else {
                continue;
            }
        }

        if( QFile::exists(filePath)) {
            if( !QFile::remove( filePath ) )
                Logger() << __FILE__ << __LINE__ << "Cannot remove file:" << filePath;
        }

        // Файлы сверяются с checkstorage при составлении списка локальных файлов из директорий
        bool res = false;
        QString action;
        if( REVISION == mContentType ) {
            res = file->copy( filePath );
            action = "copy";
        } else {
            res = file->rename( filePath );
            action = "move";
        }

        if( res ) {
            CheckDownloadedContent check( mBoxId );
            check.remove( mSourceList[i] );
        } else {
            Logger() <<  __FILE__ << __LINE__ << QString("Cannot %1 file to destination:").arg(action) << file->fileName();
        }

        result |= res;
    }

    return result;
}

bool Content::deleteExcessFiles()
{
    bool result = false;

    if( !QFile::exists( StationManager::Instance().mediaSync() + "/" + mBoxId + "/" + "command/command" ) ) {
        Logger() << __FILE__ << __LINE__ << "Revision file not exists, exit...";
        return result;
    }

    CheckDownloadedContent check(mBoxId);
    mStandartSourceFiles.clear();

    QScopedPointer<QFile> currFile(new QFile());
    if( MUSIC == mContentType ) {
        currFile->setFileName(StationManager::Instance().mediaSync() + "/" + mBoxId + "/" + "chanels/chanels");
        QFileInfo fi(*currFile);
        if( currFile->exists() && (check.checkRev(fi.fileName()) == CheckDownloadedContent::DOWNLOAD_OK) ) {
            collectContentFromFile( StationManager::Instance().mediaSync() + "/" + mBoxId + "/" + "chanels/chanels" );
            collectContentFromPath(mDest);
        } else {
            return result;
        }
    } else if( ADVERT == mContentType ) {
        currFile->setFileName(StationManager::Instance().mediaSync() + "/" + mBoxId + "/" + "cron.d/adverttable");
        QFileInfo fi(*currFile);
        if( currFile->exists() && (check.checkRev(fi.fileName()) == CheckDownloadedContent::DOWNLOAD_OK) ) {
            collectContentFromFile( StationManager::Instance().mediaSync() + "/" + mBoxId + "/" + "cron.d/adverttable" );
            collectContentFromDir(mDest);
        } else {
            return result;
        }
    } else if(REVISION == mContentType ) {
        QScopedPointer<QFile> sourceFile(new QFile() );
        for(int i = 0; i < mSourceList.size(); ++i ) {
            sourceFile->setFileName( sourceDirSlash() + mSourceList[i].dirNameSlash() + mSourceList[i].fileName() );
            QFileInfo fi(*sourceFile);
            if( !sourceFile->exists() && (check.checkRev(fi.fileName()) == CheckDownloadedContent::DEFAULT) ) {
                QString filePath = destinationSlash() + mSourceList[i].fileName();
                QScopedPointer<QFile> destFile(new QFile(filePath));
                if( destFile->exists() && !destFile->remove() ) {
                    Logger() << __FILE__ << __LINE__ <<"Cannot remove dest file:" << filePath;
                }
            }
        }
        return result;
    } else {
        Logger() << __FILE__ << __LINE__ << "Exit, wrong content type:" << mStringContentType[mContentType];
        return result;
    }

    QString destination = destinationSlash();
    QList<SContentInfo> list;
    for(auto &source : mSourceList ) {
        bool compare = false;
        for( int k = 0; k < mStandartSourceFiles.size(); ++k) {
            if( source.dirName() == mStandartSourceFiles.at(k).dirName() &&
                source.fileName() == mStandartSourceFiles.at(k).fileName() )
            {
                compare = true;
                mStandartSourceFiles.removeAt(k);
                break;
            }
        }

        if( !compare ) {
            if( MUSIC == mContentType ) {
                destination = destinationSlash() + source.dirNameSlash();
            }

            if( QFile::exists(destination + source.fileName()) ) {
                list.append(source);
            }
        }
    }

    if( list.isEmpty() )
        return result;

    result = true;

    for( auto &info: list ) {
        if( !QFile::remove(destination + info.fileName()) )
            Logger() << __FILE__ << __LINE__  << "Cannot remove file:" << destination + info.fileName();
    }

    return result;
}

QStringList Content::filesFromDirWithCheck(const QString &dir)
{
    QDir catalog(dir);
    QFileInfoList infoList = catalog.entryInfoList( QDir::Files, QDir::Name );
    QStringList files;
    for( int i = 0; i < infoList.size(); ++i ) {
        SContentInfo ci;
        ci.setDirName(catalog.dirName());
        ci.setFileName(infoList.at(i).fileName());
        ci.setContent(this);
        CheckDownloadedContent check(mBoxId);
        if( check.checkFile(ci) ) {
            files << infoList.at(i).fileName();
        }
    }
    return files;
}

QStringList Content::filesFromDir(const QString &dir)
{
    QDir catalog(dir);
    QFileInfoList infoList = catalog.entryInfoList( QDir::Files, QDir::Name );
    QStringList files;

    for( const auto &item:infoList) {
        files << item.fileName();
    }
    return files;
}

QStringList Content::reportsFiles()
{
    QStringList list = filesFromDir(mSourceDir);
//    TODO: Удаление файлов за текщую дату
    return list;
}

void Content::setRevision()
{
    if(!mLocal ) {
        appendInfo( "command",   "command"      );
        appendInfo( "chanels",   "chanels"      );
    }

    appendInfo( "cron.d",    "adverttable"  );
    appendInfo( "timetable", "timetable"    );
    appendInfo( "timetable", "vtimetable"   );

    if(!mLocal) {
        // Очищаем папки. Удаляем все файлы.
        for(int i =0; i < mSourceList.size(); ++i ) {
            QDir( mDest + mSourceList[i].dirName() ).removeRecursively();
        }
        mSourceDir = mBoxId;
    } else {
        mSourceDir  = StationManager::Instance().mediaSync() + "/" + mBoxId + "/";
        mDest       = StationManager::Instance().timetablePath();
    }
}


QList<SContentInfo> Content::sourceList()
{
    return mSourceList;
}


QList<SContentInfo> Content::standartFiles()
{
    return mStandartSourceFiles;
}


void Content::setDest(const QString &dir)
{
    mDest = dir;
}


QString Content::destination()
{
    return mDest;
}

QString Content::destinationSlash()
{
    if(mDest.isEmpty())
        return mDest;
    if(mDest.right(1) != "/")
        mDest += "/";

    return mDest;
}

void Content::collectContentFromFile(const QString &fileName)
{
    mSourceList.clear();
    if( mContentType == ADVERT ) {
        mSourceDir  = StationManager::Instance().advertDir();
    }
    else if( mContentType == MUSIC ) {
//        mDest       = destinationSlash() + StationManager::Instance().musicDir();
        mSourceDir  = StationManager::Instance().musicDir();
    }
    else {
        mDest.clear();
    }

    QScopedPointer<QFile> file( new QFile(fileName) );
    QFileInfo fi(*file);

    CheckDownloadedContent check(mBoxId);
    if( mContentType == ADVERT ) {
        // Директория рекламы как на сервере так и локальная
        QString dir = StationManager::Instance().advertDir();
        const int type = check.checkRev(fi.fileName());
        if( type == CheckDownloadedContent::DOWNLOAD_OK && file->open(QFile::ReadOnly) ) {
            QString advert = file->readAll();
            QStringList adList = advert.split('\n');
            QStringList adsFilesList;
            QStringList allFiles;
            while( !adList.isEmpty() ) {
                QString fileName = adList.takeFirst().section(';', 0,0);
                if(fileName.isEmpty())
                    continue;
                allFiles << fileName;
                if( !checkExistFile(dir, fileName) )
                    adsFilesList << fileName;
            } // while
            appendInfo( dir, adsFilesList );
            appendInfo( dir, allFiles, true );
        } else if( type != CheckDownloadedContent::DEFAULT) { // if file->open
            Logger() << __FILE__ << __LINE__<< "File advertable wrong or not found or read/write protected";
        }
    } else if(mContentType == MUSIC) {
        if( check.checkRev(fi.fileName()) == CheckDownloadedContent::DOWNLOAD_OK && file->open(QFile::ReadOnly) ) {
            QString channels = file->readAll();
            QStringList chList = channels.split('\n');
            while ( !chList.isEmpty() ) {
                QString ch = chList.takeFirst();
                if( ch.isEmpty() )
                    continue;
                QString dir = ch.split('=')[0];
                QString file = ch.section('/', -1);
                appendInfo( dir, file, true);
                if( !checkExistFile( dir, file ) ) {
                    appendInfo(dir, file);
                }
            } // while
        } else { // if file->open
            Logger() << __FILE__ << __LINE__<< "File channels not found or read/write protected";
        }

    } else { // Ни один тип контента не подошел
        Logger() << __FILE__ << __LINE__<< "ContentType не поддерживается:" << mContentType << mStringContentType[mContentType];
    }

    if( file->isOpen() )
        file->close();
}

void Content::collectContentFromPathWithCheck(const QString &path)
{
    if( mContentType != MUSIC ) {
        Logger() << __FILE__ << __LINE__<< "Только для типа MUSIC:" << mStringContentType[mContentType];
        return;
    }

    QDir dir(path);
    QFileInfoList dirList = dir.entryInfoList(QDir::NoDotAndDotDot | QDir::Dirs );

    for(int i = 0; i < dirList.size(); ++i) {
        QStringList files = filesFromDirWithCheck( dirList.at(i).absoluteFilePath() );
        QDir catalog(dirList.at(i).absoluteFilePath());
        appendInfo(catalog.dirName(), files);
    }
}

void Content::collectContentFromPath(const QString &path)
{
    if( mContentType != MUSIC ) {
        Logger() << __FILE__ << __LINE__<< "Только для типа MUSIC:" << mStringContentType[mContentType];
        return;
    }

    QDir dir(path);
    QFileInfoList dirList = dir.entryInfoList(QDir::NoDotAndDotDot | QDir::Dirs );

    for(int i = 0; i < dirList.size(); ++i) {
        QStringList files = filesFromDir( dirList.at(i).absoluteFilePath() );
        QDir catalog(dirList.at(i).absoluteFilePath());
        appendInfo(catalog.dirName(), files);
    }
}

void Content::collectRepots()
{
    appendInfo( QDir(mSourceDir).dirName(), reportsFiles() );
}

void Content::collectContentFromDirWithCheck(const QString &dir)
{
    QDir catalog(dir);
    QStringList files = filesFromDirWithCheck(dir);
    appendInfo( catalog.dirName(), files );
}

void Content::collectContentFromDir(const QString &dir)
{
    QDir catalog(dir);
    QStringList files = filesFromDir(dir);
    appendInfo( catalog.dirName(), files );
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// SContentInfo
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
SContentInfo::SContentInfo() :
    mContent(nullptr),
    mSize(0),
    mSizeCurrent(0)
{
    //
}

void SContentInfo::setFileName(const QString &name)
{
    mFileName = name;
}

void SContentInfo::setDirName(const QString &dirName)
{
    mDirName = dirName;
}

void SContentInfo::setServerPath(const QString &path)
{
    mServerPath = path;
}

void SContentInfo::setUrl(const QUrl &url)
{
    mUrl = url;
}

void SContentInfo::setContent(Content *content)
{
    mContent = content;
}

void SContentInfo::setSize(qint64 size)
{
    mSize = size;
}

void SContentInfo::setSizeCurrent(qint64 size)
{
    mSizeCurrent = size;
}

QString SContentInfo::fileName() const
{
    return mFileName;
}

QString SContentInfo::dirName() const
{
    return mDirName;
}

QString SContentInfo::dirNameSlash() const
{
    if( mDirName.isEmpty() )
        return mDirName;
    return mDirName + "/";
}

QString SContentInfo::serverPath() const
{
    return mServerPath;
}

QUrl SContentInfo::url() const
{
    return mUrl;
}

Content *SContentInfo::content() const
{
    return mContent;
}

qint64 SContentInfo::size() const
{
    return mSize;
}

qint64 SContentInfo::sizeCurrent() const
{
    return mSizeCurrent;
}
