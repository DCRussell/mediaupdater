#include "lmmediaupdaterservice.h"


#include <QCoreApplication>


LmMediaUpdaterService::LmMediaUpdaterService(int argc, char** argv) :
    QtService<QCoreApplication>(argc,argv,"lmmediaupdater")
{
    //
}


void LmMediaUpdaterService::start()
{
    mUpdateController.reset(new UpdateController());
}


void LmMediaUpdaterService::stop()
{
    mUpdateController.reset();
}
