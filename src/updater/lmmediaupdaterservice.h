#ifndef LMMEDIAUPDATERSERVICE_H
#define LMMEDIAUPDATERSERVICE_H

#include "updatecontroller.h"

#include <qtservice.h>

#include <QObject>
#include <QScopedPointer>

class LmMediaUpdaterService : public QObject, public QtService<QCoreApplication>
{
public:
    LmMediaUpdaterService(int argc, char** argv);

protected:
    //! @brief запустить сервис
    void start();

    //! @brief остановить сервис
    void stop();

private:
    ///
    QScopedPointer<UpdateController> mUpdateController;
};

#endif // LMMEDIAUPDATERSERVICE_H

