/*
 * Синглтон
 * Удаление логов
*/

#include "loggerdelete.h"
#include "stationmanager.h"

#include <algorithm>

#include <QTimer>
#include <QCoreApplication>
#include <QDir>
#include <QDateTime>


LoggerDelete::LoggerDelete() :
    QObject(0)
{
    connect( &timerDelete, &QTimer::timeout, this, &LoggerDelete::deleteLogs );
    // Сутки 86400 * 1000
    timerDelete.setInterval(86'400'000);
    timerDelete.start();

    QTimer::singleShot(10, Qt::VeryCoarseTimer, this, &LoggerDelete::deleteLogs );
}

LoggerDelete &LoggerDelete::Instance()
{
    static LoggerDelete singleInstance;
    return singleInstance;
}

///
/// Чтобы не удалять последние актуальные логи сохраняем последние 10 файлов с любой датой создания.
/// Мы предполагаем что для анализа логов нам нужны файлы, но файлы могут устареть и удалиться
/// сразу после включения бокса(бокс ехал долго до нас или забыли про него и теперь вспомнили),
/// так как срабатывает алгоритм по удаленияю файлов, которые старше N дней,
/// поэтому сортируем логи по дате создания и оставляем 10 файлов
///
void LoggerDelete::deleteLogs()
{
    QString dirName = qApp->applicationDirPath() + QDir::separator() + "logs";
    QDir dir(dirName);

    auto list = dir.entryInfoList( QStringList("*.log"), QDir::Files );
    auto dayX = QDateTime::currentDateTime();

    // Количество дней хранения логов
    int backDays = StationManager::Instance().loggerDeleteDays();
    // Отсчитываем дни от текущей даты
    dayX = dayX.addDays(backDays);
    // Если файлов меньше или равно 10, то вобще не надо ничего удалять -> выходим из функции
    if( list.size() <= 10 )
        return;

    // Сортируем по убыванию даты создания файлов
    std::sort( list.begin(), list.end(), [](QFileInfo &f1, QFileInfo &f2){ return f1.created() > f2.created(); } );

    // Убираем 10 самых новых файлов из списка на удаление
    for( int i = 0; i < 10; ++i ) {
        list.removeFirst();
    }

    // Удаляем файлы устаревшие логи за период
    for( auto &file : list ) {
        if( file.created() < dayX )
            dir.remove( file.fileName() );
    }
}
