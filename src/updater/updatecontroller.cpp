#include "updatecontroller.h"
#include "stationmanager.h"
#include "downloadmanager.h"
#include "logger.h"
#include "eventercontroller.h"


#ifdef DEBUG_MODE
    #define UPDATE_TIME 30000
#endif

#ifdef RELEASE_MODE
    #define UPDATE_TIME 300000
#endif


UpdateController::UpdateController(QObject *parent) : QObject(parent),
    mUpdateTimerWait(UPDATE_TIME)
{
    mDownloadManager.reset(new DownloadManager());
    connect(mDownloadManager.data(), &DownloadManager::startTimer, this, &UpdateController::startTimer);

    mUploadManager.reset( new UploadManager() );

    mUpdateTimer.reset(new QTimer());
    connect(mUpdateTimer.data(), &QTimer::timeout, this, &UpdateController::start);
    mUpdateTimer->setInterval(mUpdateTimerWait);
    mUpdateTimer->setSingleShot(true);
    mUpdateTimer->setTimerType(Qt::VeryCoarseTimer);

//    QTimer::singleShot( 0, Qt::VeryCoarseTimer, this, SLOT(start() ) );
    QTimer::singleShot( 0, this, &UpdateController::start );
}

UpdateController::~UpdateController()
{
    //
}

/**
 * @brief start   - Получаем данные FTP сервера
 */
void UpdateController::start()
{
    mAccountManager.reset(new AccountManager);
    connect( mAccountManager.data(), &AccountManager::requestReady, this, &UpdateController::accountManagerReady, Qt::QueuedConnection);
    mAccountManager->setLogin(StationManager::Instance().getRefreshLogin());
    mAccountManager->setPassword(StationManager::Instance().getRefreshPassword());
    if( !mAccountManager->request() ) {
        Logger() << __FILE__ << __LINE__ << mAccountManager->lastError();
        startTimer();
        return;
    }
}

/**
 * @brief accountManagerReady - Ответ с данными от сервера
 */
void UpdateController::accountManagerReady()
{
    if(mAccountManager->error() != AccountManager::NoError ) {
        Logger() << __FILE__ << __LINE__ << mAccountManager->lastError();
        EventerController::Instance().send("{update_error}", mAccountManager->lastError());
        startTimer();
        return;
    }

    QString host = mAccountManager->getValue("STHost");
    int port = 0;
    if( host.indexOf(':') != -1 ) {
        QStringList address = host.split(":");
        host = address[0];
        port = address[1].toInt();
    }

    LoginInfo loginInfo( QString::number( StationManager::Instance().getBoxId() ),
                         host,
                         mAccountManager->getValue("STLogin"),
                         mAccountManager->getValue("STPass"),
                         mAccountManager->getValue("STPrefix")
                         );
    if( port != 0 )
        loginInfo.setPort( port );

    if( loginInfo.boxId().toInt() < 1 ) {
        Logger() << __FILE__ << __LINE__ << "Unknown boxId:" << "#" + loginInfo.boxId();
        startTimer();
        return;
    }

    Logger() << __FILE__ << __LINE__ << mAccountManager->getValue("Username") << "#" + loginInfo.boxId();
    mDownloadManager->setLoginInfo(loginInfo);
    mDownloadManager->start();

    mUploadManager->setLoginInfo(loginInfo);
    mUploadManager->start();
}

void UpdateController::startTimer()
{
    mUpdateTimer->stop();
    mUpdateTimer->start();
}

