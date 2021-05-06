#ifndef UPDATECONTROLLER_H
#define UPDATECONTROLLER_H

#include "accountmanager.h"
#include "downloadmanager.h"
#include "uploadmanager.h"

#include <QObject>
#include <QTimer>
#include <QScopedPointer>

class UpdateController : public QObject
{
    Q_OBJECT
public:
    explicit UpdateController(QObject *parent = 0);
    ~UpdateController();


private:

    /**
     * @brief start   - Получаем данные FTP сервера
     */
    void start();

    /**
     * @brief accountManagerReady - Ответ с данными от сервера
     */
    void accountManagerReady();

    void startTimer();

private:
    /// Ожидание таймера обновления
    int                     mUpdateTimerWait;

    /// Менеджер аккаунта
    QScopedPointer<AccountManager>              mAccountManager;

    /// Таймер обновления
    QScopedPointer<QTimer>                      mUpdateTimer;

    /// Менеджер обновления
    QScopedPointer<DownloadManager>             mDownloadManager;

    QScopedPointer<UploadManager>               mUploadManager;
};

#endif // UPDATECONTROLLER_H
