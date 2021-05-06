#include "eventercontroller.h"

EventerController::EventerController() :
    QObject(0)
{
    eventer.reset(new Eventer());

    connect(this,&EventerController::s_monitoring,
                eventer.data(),&Eventer::monitoring,Qt::QueuedConnection);
    connect(this,&EventerController::s_eventer,
                eventer.data(),&Eventer::eventer,Qt::QueuedConnection);

    eventer->moveToThread(&eventerThread);

    connect(&eventerThread, &QThread::finished, eventer.data(), &QObject::deleteLater);
    eventerThread.start();
}
void EventerController::send(QString track,int type, QString result, int volume){
    emit s_monitoring(track,type,result,volume);
}
void EventerController::send(QString message,QString value){
    emit s_eventer(message,value);
}
