#include "contentmanager.h"
#include "logger.h"


#include "stationmanager.h"


ContentManager::ContentManager(QObject *parent) : QObject(parent),
    mTotalSize(0)
{
    //
}


ContentManager::~ContentManager()
{
    clear();
}

void ContentManager::setLoginInfo( const LoginInfo &loginInfo )
{
    mLoginInfo = loginInfo;
}

void ContentManager::setFilesize(const QString &file, qint64 size)
{
    if( mContentFilesMap.find(file) != mContentFilesMap.end() )
        mContentFilesMap[file].setSize(size);
    else {
        Logger() << __FILE__ << __LINE__ << "ERROR: key" << file << "not found";
    }
}

QUrl ContentManager::url()
{
    return QUrl();
}

void ContentManager::addNetworkContent(CONTENT_TYPE type)
{
    if( REVISION == type && mContentMap.isEmpty() ) {
        // пропускаем перебор по циклу и добавляем
    }
    else if ( REVISION == type && !mContentMap.isEmpty() ) {
        return;
    } else {
        if( mContentMap.contains( type ) ) {
            Logger() << __FILE__ << __LINE__ << "Дублирование типа контента";
            return;
        }
    }

    Content *content = new Content(type, mLoginInfo.boxId());
    mContentMap[type] = content;
    generateUrlList( content );
}


void ContentManager::setLocalContent()
{
    clear();

    bool local = true;

    Content *content = new Content(REVISION, mLoginInfo.boxId(), local);
    mContentMap[REVISION] = content;

    content = new Content(MUSIC, mLoginInfo.boxId(), local );
    mContentMap[MUSIC] = content;

    content = new Content(ADVERT, mLoginInfo.boxId(), local);
    mContentMap[ADVERT] = content;
}

void ContentManager::setReports()
{
    //
}

void ContentManager::syncContent()
{
    bool restart = false;

    setLocalContent();
    for( auto content: mContentMap ) {
        restart |= content->sync();
    }

    if( restart ) {
        StationManager::Instance().setPLayerRestart(restart);
    }
}


void ContentManager::appendSize(qint64 size)
{
    mTotalSize += size;
}

void ContentManager::setTotalSize(qint64 size)
{
    mTotalSize = size;
}

qint64 ContentManager::totalSize()
{
    return mTotalSize;
}

void ContentManager::generateUrlList(Content *content)
{
    if( content->sourceDir().isEmpty() ) {
        Logger() << __FILE__ << __LINE__ << "Неизвестен источник скачивания контента (Не задана папка для скачки с FTP).";
        return;
    }

    QString boxFolder;
    if( REVISION == content->type() )
        boxFolder = mLoginInfo.boxFolder();

    for(int i = 0; i < content->sourceList().size(); ++i ) {

        SContentInfo contentInfo = content->sourceList()[i];
        QString file = contentInfo.fileName();

        QUrl url;
        url.setScheme( mLoginInfo.scheme() );
        url.setHost( mLoginInfo.host() );

        QString path = mLoginInfo.prefixSlash();
        if( !boxFolder.isEmpty() )
            path += mLoginInfo.boxFolderSlash();
        path += content->sourceDirSlash();
        if( !boxFolder.isEmpty() )
            path += contentInfo.dirNameSlash();
        path += file;

        url.setPath( path );
        url.setPort( mLoginInfo.port() );
        url.setUserName( mLoginInfo.login() );
        url.setPassword( mLoginInfo.password() );

        contentInfo.setUrl(url);
        contentInfo.setServerPath(path);

        mContentFilesMap[contentInfo.fileName()] = contentInfo;
        if( REVISION != content->type() ) {
            mStandartFiles[content->type()] = content->standartFiles();
        }
    }
}

QList<SContentInfo> ContentManager::contentFiles()
{
    return mContentFilesMap.values();
}


//Content *ContentManager::content(CONTENT_TYPE type)
//{
//    return mContentMap[type];
//}

void ContentManager::clear()
{
    for( auto content: mContentMap ) {
        if( content )
            delete content;
    }

    mContentMap.clear();
    mContentFilesMap.clear();
}

void ContentManager::fullClear()
{
    clear();
    mStandartFiles.clear();
}

