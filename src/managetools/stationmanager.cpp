#include <QSettings>
#include "stationmanager.h"

#include "logger.h"

#include <QCryptographicHash>
#include <QCoreApplication>


StationManager::StationManager() :
    QObject(0),
    nameStation("NO SET"), stationId(LOCAL_ID),
    pathToStation(""), lastErrorStr("Data not load"),
    downloadTimeoutKey("download_timeout"),
    ftpTimeoutKey("ftp_timeout"),
    bufferSizeKey("buffer_size"),
    loggerDaysKey("loggerDeleteDays"),
    defaultLoggerDays(10),
    minimumLoggerDays(1)

{
    update();
}

StationManager &StationManager::Instance()
{
    static StationManager singleInstance;
    return singleInstance;
}

bool StationManager::update()
{
#ifdef Q_OS_WIN
    mDebugLevel = 0;//no debug
    settings.reset(new QSettings("HKEY_LOCAL_MACHINE\\SOFTWARE\\Rumediasoft\\Station", QSettings::NativeFormat));
    if( settings->status() == QSettings::NoError){
        pathToStation.setPath( settings->value("Path").toString() );

        QString conconf = qApp->applicationDirPath() + QDir::separator() +"connection.ini";
        settingsConnection.reset( new QSettings(conconf, QSettings::IniFormat) );

        QString tmp = pathToStation.absoluteFilePath("mediabox.conf");
        settingsMediabox.reset( new QSettings(tmp, QSettings::IniFormat) );

        if( settingsMediabox->status() == QSettings::NoError){

            stationId = settingsMediabox->value("mediastation/mediabox_id", LOCAL_ID).toInt();
            QVariant mediaPathData = settingsMediabox->value("mediastation/media");

            pathToMedia.setPath( mediaPathData.toString() );
            nameStation = settingsMediabox->value("mediastation/mediabox_name").toString();
            mDebugLevel = settingsMediabox->value("mediastation/debuglevel").toInt();
            QVariant vLogin = settingsMediabox->value("acid");
            QByteArray dLogin = QByteArray::fromBase64(vLogin.toByteArray());
            login = QString(dLogin);
            QVariant passw = settingsMediabox->value("acidh" );
            QByteArray dPass = QByteArray::fromBase64(passw.toByteArray());
            password = QString(dPass);

            if(pathToStation.absolutePath().isEmpty()){
                lastErrorStr = tr("<b>LampStation</b> был удален или поврежден.<br>"
                                  "Для востановления необходимо переустановить <b>LampStation</b>.<br>");
                return false;
            }
        } else {
            lastErrorStr = tr("<b>mediabox.conf</b> был удален или поврежден.<br>"
                              "Для востановления необходимо переустановить <b>lampplayer</b>.<br>");
            return false;
        }
    } else {
        lastErrorStr = tr("<b>LampStation</b> не установлен.");
        return false;
    }

#elif defined Q_OS_LINUX || defined Q_OS_OSX
    // Пути .rumediasoft
    settings.reset(new QSettings(qApp->applicationDirPath()+"/.rumediasoft/lampplayer.conf", QSettings::IniFormat ));
    //    QLOG_DEBUG() << "Conf path:" << settings->fileName();
    if( settings->status() == QSettings::NoError){
        QString path = settings->value("Path").toString() ;

        if( path.isEmpty() ) {
            path = qApp->applicationDirPath()+ "/.rumediasoft/";
            pathToStation.setPath( path);
            settings->setValue("Path", pathToStation.absolutePath());
        }

        pathToStation.setPath( path);
        settingsMediabox.reset( new QSettings(pathToStation.absoluteFilePath(qApp->applicationDirPath()+"/.rumediasoft/mediabox.conf"), QSettings::IniFormat) );

        QString conconf = pathToStation.absoluteFilePath( pathToStation.absoluteFilePath(qApp->applicationDirPath()+"/.rumediasoft/connection.ini" ) );
        settingsConnection.reset( new QSettings(conconf, QSettings::IniFormat) );

        if( settingsMediabox->status() == QSettings::NoError){

            stationId = settingsMediabox->value("mediastation/mediabox_id", LOCAL_ID).toInt();
            QVariant mediaPathData = settingsMediabox->value("mediastation/media");
            if(mediaPathData.isNull()){
                settingsMediabox->setValue("mediastation/media",pathToStation.absoluteFilePath("media"));
            }
            pathToMedia.setPath( mediaPathData.toString() );

            nameStation = settingsMediabox->value("mediastation/mediabox_name").toString();

            settingsMediabox->sync();

            QVariant vLogin = settingsMediabox->value("acid");
            QByteArray dLogin = QByteArray::fromBase64(vLogin.toByteArray());
            login= QString(dLogin);
            QVariant passw = settingsMediabox->value("acidh" );
            QByteArray dPass = QByteArray::fromBase64(passw.toByteArray());
            password= QString(dPass);


            if(pathToStation.absolutePath().isEmpty()){
                lastErrorStr = tr("<b>LampStation</b> был удален или поврежден.<br>"
                                  "Для востановления необходимо переустановить <b>LampStation</b>.<br>");
                return false;
            }
        } else {
            lastErrorStr = tr("<b>LampStation</b> был удален или поврежден.<br>"
                              "Для востановления необходимо переустановить <b>LampStation</b>.<br>");
            return false;
        }
    } else {
        lastErrorStr = tr("<b>LampStation</b> не установлен.");
        return false;
    }
#endif
    settings->sync();
    return true;
}

void StationManager::saveLogin(QString login)
{
    if(!settingsMediabox.isNull()){
        QByteArray bLogin = login.toLocal8Bit();
        this->login = login;
        settingsMediabox->setValue("acid", bLogin.toBase64());
        settingsMediabox->sync();
    }
}

void StationManager::savePassword(QString password)
{
    if(!settingsMediabox.isNull()) {
        QByteArray md5pass =  QCryptographicHash::hash( password.toUtf8(), QCryptographicHash::Md5 ).toHex();
        this->password = QString(md5pass);

        if( password.isEmpty() )
            settingsMediabox->setValue( "acidh", password );
        else
            settingsMediabox->setValue( "acidh", md5pass.toBase64() );

        settingsMediabox->sync();
    }
}

void StationManager::setDownloadTimeout(int timeout)
{
    settingsConnection->setValue(downloadTimeoutKey, timeout);
    settingsConnection->sync();
}

void StationManager::setFtpTimeout(int timeout)
{
    settingsConnection->setValue(ftpTimeoutKey, timeout);
    settingsConnection->sync();
}

void StationManager::setBufferSize(int size)
{
    settingsConnection->setValue(bufferSizeKey, size);
    settingsConnection->sync();
}

void StationManager::setLogDeleteDays(int days)
{
    if(days < minimumLoggerDays )
        days = defaultLoggerDays;

    settingsMediabox->setValue(loggerDaysKey, days);
    settingsMediabox->sync();
}

int StationManager::loggerDeleteDays()
{
    settingsMediabox->sync();
    int days = settingsMediabox->value( loggerDaysKey, defaultLoggerDays ).toInt();

    if(days < minimumLoggerDays )
        days = defaultLoggerDays;

    return days;
}

int StationManager::downloadTimeout()
{
    settingsConnection->sync();
    return settingsConnection->value(downloadTimeoutKey).toInt();
}

int StationManager::ftpTimeout()
{
    settingsConnection->sync();
    return settingsConnection->value(ftpTimeoutKey).toInt();
}

int StationManager::bufferSize()
{
    settingsConnection->sync();
    return settingsConnection->value(bufferSizeKey).toInt();
}

QString StationManager::getLogin()
{
    settingsMediabox->sync();
    login = QByteArray::fromBase64( settingsMediabox->value("acid").toByteArray() );
    return login;
}


QString StationManager::getPassword()
{
    settingsMediabox->sync();
    password = QByteArray::fromBase64( settingsMediabox->value("acidh").toByteArray() );
    return password;
}


QString StationManager::getPathToStation() const
{
    QString tmp = pathToStation.absolutePath();

    return tmp;
}

QString StationManager::get(QString subdir)
{
    return pathToStation.absoluteFilePath(subdir);
}

QString StationManager::playerPath(QString subdir)
{
    return pathToStation.absoluteFilePath(subdir);
}

QString StationManager::timetablePath()
{
    return playerPath("timetable");
}

/**
 * @brief StationManager::getRefreshLogin       - Возвращает логин
 * перечитывает каждый раз из файла при обращении к функции
 * @return
 */
QString StationManager::getRefreshLogin()
{
    settingsMediabox->sync();
    return settingsMediabox->value("acid").toString();
}

/**
 * @brief StationManager::getRefreshPassword    - Возвращает пароль
 * перечитывает каждый раз из файла при обращении к функции
 * @return
 */
QString StationManager::getRefreshPassword()
{
    settingsMediabox->sync();
    return settingsMediabox->value("acidh" ).toString();
}

int StationManager::id(){
    return stationId;
}

/** Возвращает идентификатор медиабокса
 * каждый раз перечитывает из файла
 */
int StationManager::getBoxId()
{
    settingsMediabox->sync();
    return settingsMediabox->value("mediastation/mediabox_id", LOCAL_ID).toInt();
}

void StationManager::setBoxId(int id){
    stationId = id;
    if(settingsMediabox->isWritable()){
        settingsMediabox->setValue("mediastation/mediabox_id", id);
        settingsMediabox->sync();
    } else{
        //        QLOG_ERROR() << "Cannot write to reg";
    }
}

void StationManager::saveBoxId(int id)
{
    setBoxId(id);
}


/**
 * @brief advertDir     - Директория рекламы. Локальная и на сервере совападают
 * @return
 */
QString StationManager::advertDir()
{
    return "ads";
}

QString StationManager::musicDir()
{
    return "music";
}

QString StationManager::videoDir()
{
    return "video";
}


QString StationManager::lastError()
{
    return lastErrorStr;
}

void StationManager::setRestart(const QString &key, bool state)
{
    if( "player" == key )
        settingsMediabox->setValue("playerRestart", QVariant(state) );
    else if( "mediaupdater" == key )
        settingsMediabox->setValue("mediaupdaterRestart", QVariant(state) );

    settingsMediabox->sync();
}

void StationManager::setPLayerRestart(bool state)
{
    setRestart("player", state);
}

void StationManager::setMediaUpdaterRestart(bool state)
{
    setRestart("mediaupdater", state);
}

bool StationManager::playerRestartState()
{
    settingsMediabox->sync();
    return settingsMediabox->value("playerRestart", false).toBool();
}

bool StationManager::mediaUpdaterRestartState()
{
    settingsMediabox->sync();
    return settingsMediabox->value("mediaupdaterRestart", false).toBool();
}

void StationManager::saveHash(const QString &hash)
{
    settings->setValue("machine_hash", hash);
}

QString StationManager::readHash()
{
    return settings->value("machine_hash").toString();
}

QString StationManager::media(QString subdir)
{
    return pathToMedia.absoluteFilePath(subdir);
}

QString StationManager::mediaSync()
{
    return  pathToMedia.absoluteFilePath(".sync");
}

QString StationManager::reportsDir()
{
    return pathToMedia.absoluteFilePath("reports");
}

int StationManager::revisionLocal()
{
    QDir syncPath(media(".sync"));
    QSettings settings(syncPath.absoluteFilePath("local"), QSettings::IniFormat);
    if( settings.status() == QSettings::NoError){
        int revMusicBox = settings.value("lp/id", 0 ).toInt();
        if(revMusicBox == id()){
            return settings.value("lp/revision", 0).toInt();
        }
    }
    return 0;
}


int StationManager::revisionServer()
{
    QDir syncPath(media(".sync"));
    QSettings settings(syncPath.absoluteFilePath("command"), QSettings::IniFormat);
    if( settings.status() == QSettings::NoError){
        return settings.value("/revision", 0).toInt();
    }
    return 0;
}
//запросить уровень отладки
int StationManager::getDebugLevel() const {
    return mDebugLevel;
}

int StationManager::revLocal(QString boxId)
{
    QDir syncPath(media(".sync/" +  boxId) );
    QSettings settings(syncPath.absoluteFilePath("local"), QSettings::IniFormat);
    if( settings.status() == QSettings::NoError)
        return settings.value("/revision", 0).toInt();
    return 0;
}

void StationManager::setRevLocal(int rev, QString boxId)
{
    QDir syncPath(media(".sync/" + boxId) );
    QSettings settings(syncPath.absoluteFilePath("local"), QSettings::IniFormat);
    settings.setValue("/revision", rev);
}

int StationManager::revServer(QString boxId)
{
    QDir syncPath(media(".sync/" + boxId) );
    QSettings settings(syncPath.absoluteFilePath("command/command"), QSettings::IniFormat);
    if( settings.status() == QSettings::NoError){
        return settings.value("/revision", 0).toInt();
    }
    return 0;
}
