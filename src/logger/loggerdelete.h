/*
 * Синглтон
 * Удаление логов
*/

#ifndef LOGGERDELETE_H
#define LOGGERDELETE_H

#include <QObject>
#include <QTimer>


class LoggerDelete : public QObject
{
    Q_OBJECT

    LoggerDelete();
    LoggerDelete(const LoggerDelete& root);
    LoggerDelete& operator=(const LoggerDelete&);

public:
    static LoggerDelete& Instance();

private:
    /**
     * @brief deleteLogs
     */
    void deleteLogs();

    /// Таймер для удаления устаревших логов
    QTimer      timerDelete;

};

#endif // LOGGERDELETE_H
