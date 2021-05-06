
#include "lmmediaupdaterservice.h"
#include "updatecontroller.h"
#include "logger.h"

#include <signal.h>
#include <iostream>

#include <QTextCodec>


void handler_sigsegv(int signum)
{
    Logger() << "SIGSEGV Error!";
    Logger() << "Попытка обращения к несуществующей памяти или обращения с нарушением прав доступа";
    // открепить обработчик и явно завершить приложение
    signal(signum, SIG_DFL);
    exit(3);
}


void handler_sigfpe(int signum)
{
    Logger() << "SIGFPE Error!";
    Logger() << "Выполнения ошибочной арифметической операции";
    // открепить обработчик и явно завершить приложение
    signal(signum, SIG_DFL);
    exit(3);
}

void handler_sigill(int signum )
{
    Logger() << "SIGILL Error!";
    Logger() << "Попытка выполнить неправильно сформированную, несуществующую или привилегированную инструкцию";
    signal(signum, SIG_DFL);
    exit(3);
}


int main(int argc, char *argv[])
{
    signal( SIGSEGV, handler_sigsegv );
    signal( SIGFPE,  handler_sigfpe  );
    signal( SIGILL,  handler_sigill  );

#ifdef RELEASE_MODE
    LmMediaUpdaterService service(argc, argv);
    QTextCodec *codec = QTextCodec::codecForName("UTF-8");
    QTextCodec::setCodecForLocale(codec);
    return service.exec();
#elif defined DEBUG_MODE
    QCoreApplication app(argc, argv );
    UpdateController controller;
    return app.exec();
#endif
}
