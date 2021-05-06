#include "accountmanager.h"

#include <QStringList>

#include <QNetworkRequest>
#include <QNetworkReply>

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QJsonArray>
#include <QMutexLocker>


AccountManager::AccountManager() : QObject(0),
    mCurrentReply(nullptr),
    mRequestTimeout(false)
{
    manager.reset(new QNetworkAccessManager);
    localConnect.reset( new SettingsStore(ENCRYPTION_KEY, "account.dat"));
    connect( manager.data(), SIGNAL(finished(QNetworkReply*)), this, SLOT(replyFinished(QNetworkReply*)) );
}

bool AccountManager::request()
{
    mJsonData.clear();
    mRequestTimeout = false;
    mError = NoError;

    if( mLogin.isEmpty() || mPassword.isEmpty() ) {
        mLastError = "Login or password is empty.";
        mError = LoginPasswordError;
        return false;
    }

    QString hash;
    if( !mHash.isEmpty() ) {
        hash = "&hash="+mHash;
    }

    QString url("http://account2.lampmusic.ru/verify1.php?acid=%1=&acidh=%2" + hash);
    QNetworkRequest req(QUrl(url.arg(mLogin, mPassword)) );

    mReqTimeoutTimer.setSingleShot(true);
    mReqTimeoutTimer.setInterval(5000);
    connect( &mReqTimeoutTimer, &QTimer::timeout, this, &AccountManager::requestTimeout, Qt::UniqueConnection);

    mCurrentReply = manager->get(req );
    mReqTimeoutTimer.start();
    return true;
}


void AccountManager::replyFinished(QNetworkReply* reply)
{
    QMutexLocker lock(&mMutex);
    mReqTimeoutTimer.stop();

    if( reply->error() == QNetworkReply::NoError && !mRequestTimeout ) {
        QByteArray replyData = reply->readAll();
        if( replyData.isEmpty() ) {
            mLastError = "WARNING: Server answer is empty.";
            mError = ReplyIsEmpty;
        } else {
            QByteArray decodeString = QByteArray::fromBase64( replyData );
            parseJson( decodeString );
            if( NoError == mError) {
                // Сохранение данных в кэш
                saveCache(decodeString);
            }
        }
    }

    // Проверка на ошибки ответа
    if( reply->error() != QNetworkReply::NoError || mRequestTimeout  )
    {
        QByteArray decodeString = tryLoadCache();
        if( decodeString.isEmpty() ) {
            mLastError = "WARNING: Cache is empty. ReplyError:" + reply->errorString();
            mError = ReplyError;
        } else{
            // Если парсинг из кэша проходит, то считается что никаких ошибок не было,
            // в этом случае условие NoError == mError верно
            parseJson( decodeString );
        }
    }

    reply->deleteLater();
    lock.unlock();

    emit requestReady();
}


QString AccountManager::lastError()
{
    return mLastError;
}

/**
 * @brief parseJson         - Разбор строки json
 * @param json  - массив данных
 */
void AccountManager::parseJson(const QByteArray &json)
{
    QJsonParseError jsonError;
    QJsonDocument jsonDocument ( QJsonDocument::fromJson( json, &jsonError ) );
    if( QJsonParseError::NoError != jsonError.error ) {
        mLastError = "Parse JSON error: " + jsonError.errorString();
        mError = ParseJsonError;
    } else {
        QJsonObject jsonObject;
        QJsonArray jsonArray = jsonDocument.array();
        if( jsonArray.isEmpty() ) {
            jsonObject = jsonDocument.object();
        }
        else {
            QJsonValue jsonValue = jsonArray.first();
            jsonObject = jsonValue.toObject();
        }

        foreach (auto key, jsonObject.keys() ) {
            mJsonData[key.toLower()] = jsonObject.value(key).toString();
        }

        auto iterator = mJsonData.find("error");
        if( iterator != mJsonData.end() ) {
            mLastError = iterator.value();
            mError = AnswerJsonError;
        }
    }
}


void AccountManager::setLogin(const QString &login)
{
    mLogin = login;
}

void AccountManager::setPassword(const QString &pass)
{
    mPassword = pass;
}

/**
 * @brief userId            - Возвращает идентификтор пользователя
 * @return
 */
int AccountManager::userId()
{
    return checkValue("id").toInt();
}


/**
 * @brief isActive          - активен пользователь или заблокирован сервером
 * @return
 */
int AccountManager::isActive()
{
    return checkValue("active").toInt();
}

/**
 * @brief countOrgsMax          - количество доступных организаций пользователя
 * @return      - количество
 */
int AccountManager::countOrgsMax()
{
    return checkValue("orgsbox").toInt();
}
/**
 * @brief countBoxsMax          -  количетсво доступных боксов у пользователя
 * @return      - количество
 */
int AccountManager::countBoxsMax()
{
    return checkValue("boxcount").toInt();
}


/**
 * @brief countSelect           - Возвращает количество записей, текущего пользователя, из таблицы
 * @param table             - Имя таблицы
 * @return
 */
int AccountManager::countSelect(const QString &table)
{
    Q_UNUSED(table)
    int result = -1;
    return result;
}

/**
 * @brief countOrgs             -  текущие количество организаций пользователя
 * @return      - количество
 */
int AccountManager::countOrgs()
{
    return countSelect("UsersOrgs");
}

/**
 * @brief countBoxs             - текущие количество боксов у пользователя
 * @return      - количество
 */
int AccountManager::countBoxs()
{
    return countSelect("UsersBoxs");
}

/**
  * @brief getValue             - Возвращает значение выбранного параметра
  * @param param
  * @return
  */
QString AccountManager::getValue(const QString &key)
{
    return checkValue(key);
}


QString AccountManager::checkValue(const QString &key)
{
    QString value;
    auto iterator = mJsonData.find(key.toLower() );
    if( iterator != mJsonData.end() ) {
        value = iterator.value();
    }
    else {
        //todo: check cache
        value = QString::null;
    }

return value;
}

int AccountManager::errorId()
{
    return checkValue("error_id").toInt();
}


AccountManager::ERRORS AccountManager::error()
{
    return mError;
}

QString AccountManager::login(){
    return mLogin;
}
QString AccountManager::password(){
    return mPassword;
}

void AccountManager::setHash(const QString &hash)
{
    mHash = hash;
}

/**
 * @brief saveCache - сохраняем json с сервера локально
 * @param json - массив данных с сервера
 */
void AccountManager::saveCache(const QByteArray &json)
{
    localConnect->setGroupKey("login");
    localConnect->setValue("prefs",json.toBase64());
}

/**
 * @brief tryLoadCache - пытаемся воспользоваться кешэм
 * @return если нет кеша пустой массив
 */
QByteArray AccountManager::tryLoadCache(){
    if(localConnect.isNull())
        return QByteArray();
    return  QByteArray::fromBase64(localConnect->value("login/prefs").toLocal8Bit());
}

void AccountManager::requestTimeout()
{
    mRequestTimeout = true;
    if( mCurrentReply ) {
        mCurrentReply->abort();
        mCurrentReply = nullptr;
    }
}

