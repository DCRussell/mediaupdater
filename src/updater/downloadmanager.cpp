
#include "downloadmanager.h"
#include "checkdownloadedcontent.h"
#include "logger.h"

#include "stationmanager.h"
#include "eventercontroller.h"


#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>
#include <QFile>
#include <QDir>
#include <QFileInfo>
#include <QMutexLocker>


DownloadManager::DownloadManager( QObject *parent) : QObject(parent),
    mRevisionFile("command"),
    mRevisionDir("command"),
    mBufferSize(0/*16384*/),
    mCurrentReply(nullptr),
    mFtpTimerWait(25000),
    mTotalBytes(0),
    mRevision(true),
    mDownloadWait(180000),
    mDownloadTimeout(false),
    mFtpTimeout(false),
    mFullDownloadBytes(false),
    mCurrentNumbersContentFiles(0)
{
    StationManager::Instance();
    // Создаем ContentManager в конструторе чтобы сделать setLoginInfo
    mContentManager.reset( new ContentManager() );

    //
    mNetworkManager.reset(new QNetworkAccessManager() );

    // Таймер таймаута для запросов размера файлов
    mFtpTimer.reset( new QTimer() );
    mFtpTimer->setInterval(mFtpTimerWait);
    mFtpTimer->setSingleShot(true);
    connect( mFtpTimer.data(), &QTimer::timeout, this, &DownloadManager::ftpTimeout );

    // Таймер таймаута для скачивания файлов
    mDownloadTimer.reset( new QTimer() );
    mDownloadTimer->setInterval(mDownloadWait);
    mDownloadTimer->setSingleShot(true);
    connect( mDownloadTimer.data(), &QTimer::timeout, this, &DownloadManager::downloadTimeout );
}

DownloadManager::~DownloadManager()
{
    if(mCurrentReply)
        mCurrentReply->deleteLater();
}

void DownloadManager::setLoginInfo(const LoginInfo &loginInfo)
{
    mLoginInfo = loginInfo;
    mContentManager->setLoginInfo(loginInfo);
}

void DownloadManager::start()
{
    checkConnectionSettings();

    EventerController::Instance().send("{update_progress}", "");

    mContentManager->fullClear();
    mContentManager->addNetworkContent(REVISION);
    mRevision = true;
    mContentFiles.clear();

    getTotalSize();
}

void DownloadManager::checkConnectionSettings()
{
    StationManager &manager = StationManager::Instance();

    // buffer size
    const int bf = manager.bufferSize();
    // ftp timeout
    const int ft = manager.ftpTimeout();
    // download timeout
    const int dt = manager.downloadTimeout();


    if( bf < 0) {
        mBufferSize = 0;
    } else {
        mBufferSize = bf;
    }
    manager.setBufferSize(mBufferSize);


    if( 0 == ft ) {
        //
    } else if( ft < 1000 ) {
        mFtpTimerWait = 1000;
    } else {
        mFtpTimerWait = ft;
    }
    manager.setFtpTimeout(mFtpTimerWait);

    if( dt == 0 ) {
        //
    } else if( dt < 1000 ) {
        mDownloadWait = 1000;
    } else {
        mDownloadWait = dt;
    }
    manager.setDownloadTimeout(mDownloadWait);

    mFtpTimer->setInterval(mFtpTimerWait);
    mDownloadTimer->setInterval(mDownloadWait);
}

void DownloadManager::startContent()
{
    mContentManager->clear();
    mContentManager->addNetworkContent(ADVERT);
    mContentManager->addNetworkContent(MUSIC);
    mRevision = false;
    mContentFiles.clear();
    mCurrentNumbersContentFiles = 0;

    getTotalSize();
}

void DownloadManager::nextDownload()
{
    if( mContentFiles.isEmpty() ) {
        if( mRevision ) {
            startContent();
        } else {
            CheckDownloadedContent check(mLoginInfo.boxId());

            if( check.checkAllFiles( mContentManager->contentFiles() ) && checkRevision() ) {
                StationManager::Instance().setRevLocal(StationManager::Instance().revServer(mLoginInfo.boxId()), mLoginInfo.boxId());
            }

            EventerController::Instance().send("{update_finished}", "Загрузка файлов завершена");
            mContentManager->syncContent();
            emit startTimer();
            EventerController::Instance().send("{update_success}", "Обновление файлов завершено");
            EventerController::Instance().send("{revision}", QString::number( StationManager::Instance().revLocal(mLoginInfo.boxId()) ) );
        }
        return;
    }

    Logger() << __FILE__ << __LINE__<< "Files left:" << mContentFiles.size() << "Current file:" << mContentFiles.first().fileName() << mContentFiles.first().size();

    if(!mRevision) {
        EventerController::Instance().send( "{download_info}", QString("Files left: %1").arg( mContentFiles.size() )+" Current file:" + mContentFiles.first().fileName() );
    }

    mCurrentContentFile = mContentFiles.takeFirst();

    QNetworkRequest request;
    request.setUrl( mCurrentContentFile.url() );

    // Обнуляем счетчик последних скачанных байт
    // Для каждого файла свой счетчик
    mLastBytes = 0;
    // Сбрасываем признак таймаута для новой закачки
    mDownloadTimeout        = false;
    mFullDownloadBytes      = false;

    // Удаляем предыдущий запрос если он остался
    if(mCurrentReply)
        mCurrentReply->deleteLater();


    mNetworkManager.reset( new QNetworkAccessManager() );
    mCurrentReply = mNetworkManager->get(request);

    // Устанавливаем размер при котором срабатывает ReadyRead(не факт). Не точное соответсвие этому числу, может быть немного больше.
    mCurrentReply->setReadBufferSize(mBufferSize);

    connect( mCurrentReply, &QNetworkReply::readyRead,        this,   &DownloadManager::saveDownloadedFile, Qt::QueuedConnection);
    connect( mCurrentReply, &QNetworkReply::finished,         this,   &DownloadManager::downloadFinished,   Qt::QueuedConnection);
    connect( mCurrentReply, &QNetworkReply::downloadProgress, this,   &DownloadManager::downloadProgress,   Qt::QueuedConnection);
    connect( mCurrentReply, static_cast<void (QNetworkReply::*)(QNetworkReply::NetworkError)>(&QNetworkReply::error), this, &DownloadManager::replyError, Qt::QueuedConnection);

    mDownloadTimer->start();
}

/**
 * @brief saveDownloadedFile    - Сохранение файла
 */
void DownloadManager::saveDownloadedFile()
{
    QMutexLocker lock(&mMutex);

    mDownloadTimer->stop();

    QString dest = mCurrentContentFile.content()->destinationSlash() + mCurrentContentFile.dirNameSlash();

    if( !QDir().mkpath(dest) )
        Logger() << __FILE__ << __LINE__ << "WARNING: Cannot create destination path: " << dest;

    QScopedPointer<QFile> file( new QFile( dest + mCurrentContentFile.fileName() ) );
    //TODO вот тут может быть большой косяк.
    if( !file->open( QFile::Append ) ) {
        Logger() << __FILE__ << __LINE__ << "WARNING: Could not open file for appending: " << dest + mCurrentContentFile.fileName();
    }
    else {
        QByteArray array = mCurrentReply->readAll();
        if( !array.isEmpty() ) {
            const qint64 b = file->write(array);
            if( -1 == b ) {
                Logger() << __FILE__ << __LINE__ << "WARNING: Failed to add data to a file:" << dest + mCurrentContentFile.fileName();
            }
        }
    }

    if( file->isOpen() )
        file->close();
    mDownloadTimer->start();
}

/**
 * @brief DownloadManager::downloadFinished
 */
void DownloadManager::downloadFinished()
{
    QMutexLocker lock(&mMutex);

    mDownloadTimer->stop();

    QScopedPointer<QFileInfo> fileinfo( new QFileInfo( mCurrentContentFile.content()->destinationSlash() + mCurrentContentFile.dirNameSlash() + mCurrentContentFile.fileName() ));

    CheckDownloadedContent check(mLoginInfo.boxId());

    // Проверка по размеру файла не производится так как может получится, что невозможно получить размеры файла с сервера
    if( fileinfo->exists() && !mDownloadTimeout && QNetworkReply::NoError == mCurrentReply->error() ) {
        if( !mRevision ) {
            check.setTrust(mCurrentContentFile);
        } else {
            check.setRev(mCurrentContentFile, CheckDownloadedContent::DOWNLOAD_OK);
        }
    } else {
        if( mRevision ) {
            check.setRev(mCurrentContentFile, CheckDownloadedContent::DOWNLOAD_FAIL);
        }
    }

    // Если размер файла на сервере равен размеру скачанного файла,
    // то закачака была полностью завершена тогда это файл доверенный
    // независимо от того сработал таймаут или нет и были ли ошибки в QNetworkReply или нет
    if( fileinfo->exists() && !mRevision && mFullDownloadBytes  ) {
        check.setTrust(mCurrentContentFile);
    }

    if(mCurrentReply) {
        mCurrentReply->deleteLater();
        mCurrentReply = nullptr;
    }

    if(!mRevision && mContentFilesNumbers > 0) {
        int percent = (++mCurrentNumbersContentFiles * 100) / mContentFilesNumbers;
        EventerController::Instance().send( "{process_update}", QString::number(percent) );
    }

    if(!mRevision) {
        Logger() << __FILE__ << __LINE__
                 << "FileName:" << mCurrentContentFile.fileName() <<";"
                 << "Server file size:" << mCurrentContentFile.size() << ";"
                 << "Downloaded size:" << mCurrentContentFile.sizeCurrent() << ";"
                 << "Local file size:" << fileinfo->size() << ";"
                 << "File status:" << (mFullDownloadBytes ? "OK":"FAIL") << ";"
                 << "Trust:" << (check.trust(mCurrentContentFile) ? "TRUE":"FALSE");
    }

    lock.unlock();

//    QTimer::singleShot(0, Qt::VeryCoarseTimer, this, SLOT( nextDownload() ) );
    QTimer::singleShot(0, this, &DownloadManager::nextDownload );
}

void DownloadManager::downloadProgress(qint64 cb, qint64 tb)
{
    QMutexLocker lock(&mMutex);
    mDownloadTimer->start();

    if(!mRevision) {
        mCurrentBytes += cb - mLastBytes;
        mLastBytes = cb;

        // Если размер полученных данных(cb) равен(или больше) размеру файла на сервере(tb) и полученные данные больше 0(нуля),
        // а total bytes(tb) не должны быть равны -1(если размер файла заранее неизвестен),
        // то закачка завершена
        if( cb >= tb && cb > 0 && tb != -1 ) {
            QScopedPointer<QFileInfo> fileinfo( new QFileInfo( mCurrentContentFile.content()->destinationSlash() + mCurrentContentFile.dirNameSlash() + mCurrentContentFile.fileName() ));
            // Если локальный файл равен по размеру переданных данным, тогда считаем что файл полность скачен
            if( fileinfo->size() == cb ) {
                mFullDownloadBytes = true;
            }
        }
        mCurrentContentFile.setSizeCurrent(cb);
    }
}

void DownloadManager::replyError(QNetworkReply::NetworkError error)
{
    QMutexLocker lock(&mMutex);

    mDownloadTimer->stop();

    Logger() << __FILE__ << __LINE__ << "Download error:" << error << "File:" << mCurrentContentFile.fileName();
    EventerController::Instance().send( "{download_error}", "File:" + mCurrentContentFile.fileName() );
}

bool DownloadManager::checkRevision()
{
    const int revServer   = StationManager::Instance().revServer( mLoginInfo.boxId() );
    const int revLocal    = StationManager::Instance().revLocal( mLoginInfo.boxId() );
    if( revServer <= 0 )
        return false;

    if( revLocal != revServer )
        return true;

    return false;
}

void DownloadManager::getTotalSize()
{
    mFtp.reset(new QFtp());
    mFtpContentFiles.clear();
    mFtpContentFiles.append( mContentManager->contentFiles() );
    if( mFtpContentFiles.isEmpty() ) {
        QTimer::singleShot(0, this, &DownloadManager::nextDownload );
        return;
    }

    mTotalBytes      = 0;
    mLastBytes       = 0;
    mCurrentBytes    = 0;
    mContentManager->setTotalSize(0);

    if( !connectToFtp() ) {
        emit startTimer();
        return;
    }
}

void DownloadManager::nextFtpFile()
{
    if( mFtpContentFiles.isEmpty() ) {
        mFtp->close();
        Logger() << __FILE__ << __LINE__<< "Total size=" << mTotalBytes << "bytes";
        mContentFilesNumbers = mContentFiles.size();
        QTimer::singleShot(0, this, &DownloadManager::nextDownload );
        return;
    }

    mCurrentContentFile = mFtpContentFiles.takeFirst();
    getFileSize( mCurrentContentFile.serverPath() );
}


void DownloadManager::getFileSize(const QString &file)
{
    if( !file.isEmpty() ) {
        mFtpTimeout = false;
        mFtpTimer->start();
        mFtp->list( file );
    } else {
        nextFtpFile();
    }
}

void DownloadManager::listInfo(const QUrlInfo &info)
{
    QMutexLocker lock(&mMutex);
    mFtpTimer->stop();

    if( mFtpTimeout )
        return;

    if( info.isValid() && info.isFile() ) {
        mTotalBytes += info.size();
        mCurrentContentFile.setSize( info.size() );
        mContentFiles.append(mCurrentContentFile);
        mContentManager->appendSize( info.size() );
    }
    lock.unlock();

    nextFtpFile();
}


bool DownloadManager::connectToFtp()
{
    if( mLoginInfo.host().isEmpty() || mLoginInfo.login().isEmpty() || mLoginInfo.password().isEmpty() ) {
        Logger() << __FILE__ << __LINE__ << "Host or Login or Passwrod is Empty";
        return false;
    }

    connect( mFtp.data(), &QFtp::commandFinished,   this, &DownloadManager::ftpCommandFinished, Qt::UniqueConnection);
    connect( mFtp.data(), &QFtp::listInfo,          this, &DownloadManager::listInfo, Qt::UniqueConnection);

    mFtpTimeout = false;
    mFtpTimer->start();
    mFtp->connectToHost( mLoginInfo.host(), mLoginInfo.port() );
    mLoginCommand = mFtp->login(mLoginInfo.login(), mLoginInfo.password());

    return true;
}

void DownloadManager::ftpCommandFinished(int id, bool error)
{
    QMutexLocker lock(&mMutex);

    if( mFtpTimeout )
        return;

    if( error ) {
        mFtpTimer->stop();
        if( QFtp::UnknownError == mFtp->error() || QFtp::NoError == mFtp->error() ) {
            lock.unlock();
            nextFtpFile();
            return;
        }

        Logger() << __FILE__ << __LINE__ << "FTP Server Error:" << mFtp->errorString();
        EventerController::Instance().send( "{ftp_error}", mFtp->errorString() );
        emit startTimer();
        return;
    }

    if( mFtp->state() == QFtp::Connected) {
        Logger() << "Connected OK";
    } else if( mFtp->state() == QFtp::LoggedIn ) {
        if(mLoginCommand == id) {
            mFtpTimer->stop();
            Logger() << "Login OK";
            lock.unlock();
            nextFtpFile();
        }
    }
}

void DownloadManager::ftpTimeout()
{
    mFtpTimeout = true;
    Logger() << __FILE__ << __LINE__ << QString("Ftp timeout. The server does not respond within %1 ms").arg(mFtpTimerWait);
    EventerController::Instance().send( "{ftp_timeout}", "FTP-сервер не отвечает" );
    mFtp->close();
    emit startTimer();
}

void DownloadManager::downloadTimeout()
{
    if( mCurrentReply->bytesAvailable() ) {
        Logger() << __FILE__ << __LINE__ << "FOUND SOME BYTES...";
        saveDownloadedFile();
        return;
    }

    Logger() <<  __FILE__ << __LINE__ <<  QString("Download timeout. The server does not respond within %1 ms").arg(mDownloadWait);
    EventerController::Instance().send( "{download_timeout}", "File:" + mCurrentContentFile.fileName() );
    mDownloadTimeout = true;
    if( mCurrentReply )
        mCurrentReply->abort();
}

