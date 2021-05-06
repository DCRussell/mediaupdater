#ifndef STATIONMANAGER_H
#define STATIONMANAGER_H

#include <QObject>
#include <QDir>
#include <QMap>
#include <QSettings>
#include <QScopedPointer>

#define LOCAL_ID -1


class StationManager : public QObject
{
    Q_OBJECT
private:
    StationManager(const StationManager&);
    StationManager& operator=(const StationManager&);

    QString                             nameStation;
    int                                 stationId;          // if -1(LOCAL_ID) => local
    QDir                                pathToStation;
    QDir                                pathToMedia;
    QString                             lastErrorStr;
    QString                             login;
    QString                             password;
    //писать ли данные в лог, глубина отладки 0 - не писать, 1,2,3 и т.д. писать
    int                                 mDebugLevel;
    QString                             mHash;
    QScopedPointer<QSettings>           settings;
    QScopedPointer<QSettings>           settingsMediabox;
    QScopedPointer<QSettings>           settingsConnection;

    const QString                       downloadTimeoutKey;
    const QString                       ftpTimeoutKey;
    const QString                       bufferSizeKey;
    const QString                       loggerDaysKey;
    const int                           defaultLoggerDays;
    const int                           minimumLoggerDays;

public:
    StationManager();
    static StationManager& Instance();

    int getDebugLevel() const;
    //свойства для QML
    Q_PROPERTY(QString Login READ getLogin WRITE saveLogin NOTIFY loginChanged)
    Q_PROPERTY(int Id READ id WRITE setBoxId NOTIFY idChanged)

    Q_INVOKABLE QString media(QString subdir = "");
    QString mediaSync();

    /**
     * @brief reportsDir    - Возвращает путь до папки отчетов
     * @return
     */
    QString reportsDir();

    bool update();
    QString getPathToStation() const;
    QString get(QString subdir);
    QString playerPath(QString subdir = "");
    QString timetablePath();

    Q_INVOKABLE int revisionLocal();
    int revLocal(QString boxId);
    void setRevLocal(int rev, QString boxId);
    /////
    /// \brief возвращает сетевую ревизию
    ///
    Q_INVOKABLE int revisionServer();
    int revServer(QString boxId);
    /////
    /// \brief возвращает текущий логин
    ///
    Q_INVOKABLE QString getLogin();
    /////
    /// \brief возвращает текущий пароль
    ///
    ///
    Q_INVOKABLE QString getPassword();

    /**
     * @brief StationManager::getRefreshLogin       - Возвращает логин
     * перечитывает каждый раз из файла при обращении к функции
     * @return
     */
    QString getRefreshLogin();

    /**
     * @brief StationManager::getRefreshPassword    - Возвращает пароль
     * перечитывает каждый раз из файла при обращении к функции
     * @return
     */
    QString getRefreshPassword();

    /**
     * @brief saveLogin
     */
    void saveLogin(QString);

    /**
     * @brief savePassword
     */
    void savePassword(QString);

    /**
     * @brief setDownloadTimeout
     * @param timeout
     */
    void setDownloadTimeout(int timeout);

    /**
     * @brief setFtpTimeout
     * @param timeout
     */
    void setFtpTimeout(int timeout);

    /**
     * @brief setBufferSize
     * @param size
     */
    void setBufferSize(int size);

    /**
     * @brief setLogDeleteDays
     * @param days
     */
    void setLogDeleteDays( int days );
    int loggerDeleteDays();

    int downloadTimeout();
    int ftpTimeout();
    int bufferSize();

    /**
     * @brief isAlter
     * @return
     */
    bool isAlter();

    /**
     * @brief id
     * @return
     */
    int id();

    /** Возвращает идентификатор медиабокса
     * каждый раз перечитывает из файла
     */
    int getBoxId();

    /**
     * @brief setBoxId Сохраняет идентификтор
     * @param id
     */
    void setBoxId(int id);
    void saveBoxId( int id );

    /**
     * @brief advertDir     - Директория рекламы. Локальная и на сервере совападают
     * @return
     */
    QString advertDir();

    /**
     * @brief musicDir      - Директория музыки. Локальная и на сервере совападают
     * @return
     */
    QString musicDir();
    QString videoDir();

    /**
     * @brief lastError
     * @return
     */
    QString lastError();

    void setRestart(const QString &key, bool state);
    void setPLayerRestart( bool state);
    void setMediaUpdaterRestart( bool state);

    bool playerRestartState();
    bool mediaUpdaterRestartState();

    /**
     * @brief saveHash
     * @param hash
     */
    void saveHash( const QString &hash);

    /**
     * @brief readHash
     * @return
     */
    QString readHash();

signals:
    void loginChanged();
    void idChanged();
    void typeChanged();
};

//Путь до скриптов
#define STATIONPATH StationManager::Instance().getPathToStation()
//Путь до файла
#define STATIONPATHTO(X) StationManager::Instance().get(X)
//Путь до хранилища
#define STATIONMEDIA StationManager::Instance().media("")
//одинаковые
#define STATIONMEDIATO(X) StationManager::Instance().media(X)

///
#endif // STATIONMANAGER_H
