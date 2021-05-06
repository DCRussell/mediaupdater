#ifndef EVENETER_H
#define EVENETER_H

#include "accountmanager.h"

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>


class Eventer : public QObject
{
    Q_OBJECT
    QScopedPointer<QNetworkAccessManager> networkManager;
    QScopedPointer<AccountManager> account;
public:
    explicit Eventer(QObject *parent = 0);
private:
    ///
    /// \brief request - метод отправляет запрос
    /// \param uri - сам запрос
    ///
    void request(QString uri);
    ///
    /// \brief getUri -  возвращает полный путь
    /// \param suburi - под путь параметры
    /// \return - собственно путь
    ///
    QString getUri(QString suburi);
    QNetworkAccessManager* manager();

    QString serverUri;
//    int serverPort;

    void getServerUri();

private slots:
    void requestFinished(QNetworkReply *reply);

public slots:
    ///
    /// \brief monitoring -  что играет сейчас
    /// \param track - трек
    /// \param type - тип  0  музыка  1 реклама
    /// \param result - состоаяние
    /// \param volume -  громкость
    ///
    void monitoring(QString track,int type, QString result, int volume);
    ///
    /// \brief eventer - события он же логгер: в формате json
    ///  {"msg": "message","value":"value"}
    /// \param message - сообщение
    /// \param value - занчение
    ///
    ///
    void eventer(QString message,QString value);
    void dataUpdate();
};

#endif // EVENETER_H
