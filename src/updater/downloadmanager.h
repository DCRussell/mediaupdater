#ifndef DOWNNLOADMANAGER_H
#define DOWNNLOADMANAGER_H


#include "logininfo.h"
#include "contentmanager.h"
#include "content.h"

#include "qftp.h"
#include "qurlinfo.h"


#include <QObject>
#include <QStringList>
#include <QScopedPointer>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QTimer>
#include <QString>
#include <QMutex>


class DownloadManager : public QObject
{
    Q_OBJECT
public:
    explicit DownloadManager(QObject *parent = 0);
    ~DownloadManager();

    /**
     * @brief setLoginInfo
     * @param loginInfo
     */
    void setLoginInfo(const LoginInfo &loginInfo);

    /**
     * @brief getTotalSize
     */
    void getTotalSize();

    /**
     * @brief nextFtpFile
     */
    void nextFtpFile();

    /**
     * @brief getFileSize
     * @param file
     */
    void getFileSize(const QString &file);

    /**
     * @brief start
     */
    void start();

    /**
     * @brief checkConnectionSettings
     */
    void checkConnectionSettings();


private:
    void startContent();

    /**
     * @brief connectToFtp          - соединение с FTP сервером
     * @return
     */
    bool connectToFtp();

    /**
     * @brief ftpCommandFinished    - Завершение выполнение команды FTP
     */
    void ftpCommandFinished(int id, bool error);

    /**
     * @brief saveDownloadedFile    - Сохранение файла
     */
    void saveDownloadedFile();

    /**
     * @brief downloadFinished
     */
    void downloadFinished();

    /**
     * @brief nextDownload
     */
    void nextDownload();

    /**
     * @brief downloadProgress      - Прогресс закачки
     * @param bytesReceived
     * @param bytesTotal
     */
    void downloadProgress(qint64 bytesReceived, qint64 bytesTotal);

    /**
     * @brief replyError
     * @param error
     */
    void replyError( QNetworkReply::NetworkError error );

    /**
     * @brief listInfo
     * @param info
     */
    void listInfo( const QUrlInfo &info);

    /**
     * @brief ftpTimeout
     */
    void ftpTimeout();

    /**
     * @brief downloadTimeout
     */
    void downloadTimeout();

    /**
     * @brief checkRevision
     * @return
     */
    bool checkRevision();

signals:
    void startTimer();

private:
    /// Работа с FTP для получения размеров файлов
    QScopedPointer<QFtp>                        mFtp;

    /// Файл с ревизией
    QString                                     mRevisionFile;

    /// Директория файла ревизии
    QString                                     mRevisionDir;

    /// Структура с информацией о заходе на сервер FTP
    LoginInfo                                  mLoginInfo;

    /// Сетевой менеджер
    QScopedPointer<QNetworkAccessManager>       mNetworkManager;

    /// Размер буфера считывания из сетевого запроса
    int                                         mBufferSize;

    /// Текущий запрос в сеть
    QNetworkReply                               *mCurrentReply;

    /// Таймер для прерывания работы FTP если не отвечает
    QScopedPointer<QTimer>                      mFtpTimer;

    /// Счетчик для таймера FTP
    int                                         mFtpTimerWait;

    /// Объем данных необходимый для скачивания с сервера
    qint64                                      mTotalBytes;

    /// Счетчик последних скачанных байт текущего файла
    qint64                                      mLastBytes;

    /// Счетчик всех скачанных байт всех файлов
    qint64                                      mCurrentBytes;

    /// Менеджер контента
    QScopedPointer<ContentManager>              mContentManager;

    /// Список загружаемых файлов для QNetWorkAccessManager
    QList<SContentInfo>                         mContentFiles;

    /// Список загружаемых файлов для подсчета размера файлов на FTP
    QList<SContentInfo>                         mFtpContentFiles;

    /// Текущий загружаемый файл
    SContentInfo                                mCurrentContentFile;

    /// Состояние типа контента закачки менеджера
    bool                                        mRevision;

    /// Код команды логина на сервер
    int                                         mLoginCommand;

    /// Таймер ожидания ответа от сети
    QScopedPointer<QTimer>                      mDownloadTimer;

    /// Время ожидания таймера
    int                                         mDownloadWait;

    ///
    bool                                        mDownloadTimeout;
    bool                                        mFtpTimeout;
    bool                                        mFullDownloadBytes;

    int                                         mContentFilesNumbers;
    int                                         mCurrentNumbersContentFiles;


    QMutex                                      mMutex;
};

#endif // DOWNNLOADMANAGER_H
