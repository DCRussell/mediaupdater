#ifndef CONTENTMANAGER_H
#define CONTENTMANAGER_H

#include "content.h"
#include "logininfo.h"


#include <QObject>
#include <QScopedPointer>
#include <QNetworkReply>
#include <QList>
#include <QUrl>


class ContentManager : public QObject
{
    Q_OBJECT
public:
    explicit ContentManager(QObject *parent = 0);
    ~ContentManager();

    /**
     * @brief setLoginInfo
     * @param loginInfo
     */
    void setLoginInfo( const LoginInfo &loginInfo);

    /**
     * @brief setFilesize
     * @param file
     * @param size
     */
    void setFilesize( const QString &file, qint64 size);

    /**
     * @brief url
     * @return
     */
    QUrl url();

    /**
     * @brief addContent
     * @param type
     */
    void addNetworkContent(CONTENT_TYPE type);

    /**
     * @brief setLocalContent
     */
    void setLocalContent();

    /**
     * @brief setReports    - Собираем файлы отчетов
     */
    void setReports();

    /**
     * @brief syncContent
     * @return
     */
    void syncContent();

    /**
     * @brief appendSize
     * @param size
     */
    void appendSize( qint64 size );

    /**
     * @brief setTotalSize
     * @param size
     */
    void setTotalSize( qint64 size);

    /**
     * @brief totalSize
     * @return
     */
    qint64 totalSize();

    /**
     * @brief generateUrlList
     * @param content
     */
    void generateUrlList( Content *content );

    /**
     * @brief contentFiles
     * @return
     */
    QList<SContentInfo> contentFiles();

    /**
     * @brief content
     * @param type
     * @return
     */
//    Content *content(CONTENT_TYPE type);

    void clear();

    void fullClear();

private:
    /// Карта контанта
    QMap<CONTENT_TYPE, Content*>                mContentMap;

    /// Файлы для загрузки
    QMap<QString, SContentInfo>                 mContentFilesMap;

    /// Эталонные файлы которые должны быть в папках
    QMap<CONTENT_TYPE, QList<SContentInfo>>     mStandartFiles;

    /// Информация для логина на FTP сервер
    LoginInfo                                   mLoginInfo;

    /// Размер всех файлов закачки
    qint64                                      mTotalSize;
};

#endif // CONTENTMANAGER_H
