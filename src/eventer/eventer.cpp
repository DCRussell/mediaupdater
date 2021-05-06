
#include "eventer.h"
#include "stationmanager.h"

#include <QNetworkRequest>
#include <QNetworkReply>
#include <QTimer>


Eventer::Eventer(QObject *parent) :
    QObject(parent)
{
    getServerUri();
}

void Eventer::request(QString uri)
{
    manager()->get(QNetworkRequest(QUrl(uri)));
}

QNetworkAccessManager* Eventer::manager()
{
    if(networkManager.isNull()){
        networkManager.reset(new QNetworkAccessManager());
    }
    connect( networkManager.data(), &QNetworkAccessManager::finished, this, &Eventer::requestFinished);
    return networkManager.data();
}

QString Eventer::getUri(QString suburi)
{
    QString url = "%1%2";

    return url.arg(serverUri,suburi);
}

void Eventer::monitoring(QString track, int type, QString result, int volume)
{
    StationManager& manager = StationManager::Instance();
    QString uriFormat = "loger.php?date=%1&time=%2&mbid=%3&track=%4&mtype=%5&result=%6&volume=%7&mfz=0";

    QString uri = uriFormat
            .arg(QDateTime::currentDateTime().toString("yyyy-M-d"))
            .arg(QDateTime::currentDateTime().toString("h:m:s"))
            .arg( manager.getBoxId() )
            .arg(track)
            .arg(type)
            .arg(result)
            .arg(volume);
    request(getUri(uri));
}
///
/// \brief eventer - события он же логгер: в формате json
///  {"msg": "message","value":"value"}
/// \param mbid - ид бокса
/// \param message - сообщение
/// \param value - занчение
///
///
void Eventer::eventer(QString message,QString value)
{
    StationManager& manager = StationManager::Instance();
    QString uriFormat = "eventer.php?mbid=%1&process=%2&message=%3";
    QString process = "lampplayer_daemon";

    QString jsonMsg = "{\"msg\":\"%1\",\"value\":\"%2\"}";
    QString uri = uriFormat
            .arg( manager.getBoxId())
            .arg(process)
            .arg(jsonMsg
                 .arg(message)
                 .arg(value));
    request(getUri(uri));
}

void Eventer::getServerUri()
{
    if(account.isNull()){
        account.reset(new AccountManager());

        connect( account.data(), &AccountManager::requestReady, this,&Eventer::dataUpdate );

        StationManager &station = StationManager::Instance();

        QByteArray bLogin = station.getLogin().toLocal8Bit();
        QByteArray bPass = station.getPassword().toLocal8Bit();

        account->setLogin(bLogin.toBase64());
        account->setPassword(bPass.toBase64());
        account->request();
    }
    return;
}

void Eventer::requestFinished(QNetworkReply *reply)
{
    if(reply)
        reply->deleteLater();
}

void Eventer::dataUpdate()
{
    if(account.isNull())
        return;
    serverUri = account->getValue("ReportURI");

    if( serverUri.isEmpty() )
        QTimer::singleShot(300000, Qt::VeryCoarseTimer, this,  &Eventer::getServerUri);
}
