#ifndef EVENTERCONTROLLER_H
#define EVENTERCONTROLLER_H

#include <QObject>
#include <QThread>
#include <QScopedPointer>

#include "eventer.h"

class EventerController : public QObject
{
    Q_OBJECT
    QThread eventerThread;
    EventerController();
    EventerController(const EventerController&);
    EventerController& operator=(const EventerController&);

    QScopedPointer<Eventer> eventer;
public:
    static EventerController& Instance()
    {
        static EventerController singleInstance;
        return singleInstance;
    }
    void send(QString track,int type, QString result, int volume);
    void send(QString message,QString value);
signals:
    ///
    /// \brief monitoring -  что играет сейчас
    /// \param track - трек
    /// \param type - тип  0  музыка  1 реклама
    /// \param result - состоаяние
    /// \param volume -  громкость
    ///
    void s_monitoring(QString track,int type, QString result, int volume);
    ///
    /// \brief eventer - события он же логгер: в формате json
    ///  {"msg": "message","value":"value"}
    /// \param message - сообщение
    /// \param value - занчение
    ///
    ///
    void s_eventer(QString message,QString value);
public slots:

};

#endif // EVENTERCONTROLLER_H
