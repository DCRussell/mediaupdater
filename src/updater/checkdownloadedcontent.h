#ifndef CHECKDOWNLOADEDCONTENT_H
#define CHECKDOWNLOADEDCONTENT_H

#include "content.h"

#include <QObject>
#include <QSettings>
#include <QScopedPointer>

class CheckDownloadedContent : public QObject
{
    Q_OBJECT
public:
    explicit CheckDownloadedContent(QString boxId, QObject *parent = 0);

    enum CHECK_STATUS { DEFAULT, DOWNLOAD_OK, DOWNLOAD_FAIL };

    /**
     * @brief checkFile
     * @param info
     * @return
     */
    bool checkFile( const SContentInfo &info);

    int checkRev( const QString &fileName );
    int checkRev( const SContentInfo &info );

    void setRev(const QString &fileName, CHECK_STATUS st);
    void setRev( const SContentInfo &info, CHECK_STATUS st );

    /**
     * @brief checkAllFiles
     * @param infoList
     * @return
     */
    bool checkAllFiles( const QList<SContentInfo> &infoList );

    /**
     * @brief setCheckState
     * @param info
     * @param state
     */
    void setTrust(const SContentInfo &info);

    /**
     * @brief trust
     * @return
     */
    bool trust(const SContentInfo &info);

    /**
     * @brief remove
     * @param info
     */
    void remove(const SContentInfo &info );

private:
    QScopedPointer<QSettings>           mSettings;
    QScopedPointer<QSettings>           mRevSettings;
};

#endif // CHECKDOWNLOADEDCONTENT_H
