#ifndef ACCOUNTMANAGER_H
#define ACCOUNTMANAGER_H

#include "settingsstore.h"

#include <QObject>
#include <QJsonValue>
#include <QMap>
#include <QScopedPointer>
#include <QNetworkAccessManager>
#include <QMutex>
#include <QTimer>
#include <QNetworkReply>

class QNetworkReply;
const char ENCRYPTION_KEY[] = "lmHar01lov410l";

class AccountManager : public QObject
{
    Q_OBJECT
private:
    QScopedPointer<QNetworkAccessManager> manager;
    QScopedPointer<SettingsStore> localConnect;
public:
    AccountManager();
    AccountManager(const AccountManager&);
    AccountManager& operator=(const AccountManager&);

    ///
    enum ERRORS { NoError, ReplyError, ParseJsonError, AnswerJsonError, LoginPasswordError, ReplyIsEmpty };

    /**
     * @brief request
     * @return
     */
    bool request();

    /**
     * @brief lastError
     * @return
     */
    QString lastError();

    void setLogin(const QString &login);

    void setPassword(const QString &pass);

    /**
     * @brief userId            - Возвращает идентификтор пользователя
     * @return
     */
    int userId();

    /**
      * @brief getValue         - Возвращает значение выбранного параметра
      * @param param
      * @return
      */
     QString getValue(const QString &key);

    /**
     * @brief isActive          - активен пользователь или заблокирован сервером
     * @return
     */
    int isActive();

    /**
     * @brief countOrgsMax          - количество доступных организаций пользователя
     * @return      - количество
     */
    int countOrgsMax();

    /**
     * @brief countBoxsMax          -  количетсво доступных боксов у пользователя
     * @return      - количество
     */
    int countBoxsMax();

    /**
     * @brief countOrgs             -  текущие количество организаций пользователя
     * @return      - количество
     */
    int countOrgs();

    /**
     * @brief countBoxs             - текущие количество боксов у пользователя
     * @return      - количество
     */
    int countBoxs();

    ERRORS error();
    QString login();
    QString password();

    /**
     * @brief setHash
     */
    void setHash(const QString &hash);

    /**
     * @brief errorId
     * @return
     */
    int errorId();

private slots:
    /**
     * @brief replyFinished
     */
    void replyFinished(QNetworkReply*);

private:

    /**
     * @brief parseJson         - Разбор строки json
     * @param json  - массив данных
     */
    void parseJson(const QByteArray &json);

    /**
     * @brief checkValue            - Проверка доступности значения
     * @param key
     * @return
     */
    QString checkValue(const QString & key);

    /**
     * @brief countSelect           - Возвращает количество записей, текущего пользователя, из таблицы
     * @param table             - Имя таблицы
     * @return
     */
    int countSelect(const QString &table);

    /**
     * @brief saveCache - сохраняем json с сервера локально
     * @param json - массив данных с сервера
     */
    void saveCache(const QByteArray &json);
    /**
     * @brief tryLoadCache - пытаемся воспользоваться кешэм
     * @return если нет кеша пустой массив
     */
    QByteArray tryLoadCache();

    /**
     * @brief requestTimeout
     */
    void requestTimeout();

signals:
    void requestReady();


private:
    ///
    QString                             mLastError;

    ///
    QMap<QString, QString>              mJsonData;

    ///
    QString                             mLogin;

    ///
    QString                             mPassword;

    ///
    ERRORS                              mError;

    ///
    QString                             mHash;

    ///
    QMutex                              mMutex;

    /// Таймер запроса
    QTimer                              mReqTimeoutTimer;

    /// Таймаут запроса
    bool                                mRequestTimeout;

    /// Текущий запрос
    QNetworkReply                       *mCurrentReply;
};

#endif // ACCOUNTMANAGER_H
