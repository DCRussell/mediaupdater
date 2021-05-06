#ifndef CONTENT_H
#define CONTENT_H


#include <QString>
#include <QMap>
#include <QList>
#include <QUrl>

// Объявляем класс Content для использования в ScontentInfo
class Content;


// MUSIC    - Музыкальные файлы
// ADVERT   - Рекламные файлы
// REVISION - Файлы управления
enum CONTENT_TYPE { MUSIC, ADVERT, REVISION, REPORT };

struct SContentInfo {
    SContentInfo();



    void setFileName(const QString &name );
    void setDirName( const QString &dirName );
    void setServerPath( const QString &path);
    void setUrl( const QUrl &url );
    void setContent( Content *content );
    void setSize( qint64 size );
    void setSizeCurrent( qint64 size);

    QString fileName() const;
    QString dirName() const ;
    QString dirNameSlash() const;
    QString serverPath() const;
    QUrl url() const;
    Content *content() const;
    qint64  size() const;
    qint64  sizeCurrent() const;

private:
    QString             mFileName;
    QString             mDirName;
    QString             mServerPath;
    QUrl                mUrl;
    Content             *mContent;
    qint64              mSize;
    qint64              mSizeCurrent;
};

class Content
{
public:
    Content(CONTENT_TYPE type, QString boxId, bool local = false);
    ~Content();

    /**
     * @brief sourceList
     * @return
     */
    QList<SContentInfo> sourceList();
    QList<SContentInfo> standartFiles();

    /**
     * @brief localDestination
     * @return
     */
    QString destination();
    QString destinationSlash();

    /**
     * @brief setDest
     * @param dir
     */
    void setDest( const QString &dir );

    /**
     * @brief type          - Возвращает тип контента
     * @return
     */
    CONTENT_TYPE type();

    /**
     * @brief sourceDir
     * @return
     */
    QString sourceDir();

    /**
     * @brief sourceDirSlash
     * @return
     */
    QString sourceDirSlash();

    /**
     * @brief sync
     */
    bool sync();

    /**
     * @brief isLocal
     * @return
     */
    bool isLocal();

private:
    /**
     * @brief collectFromFile
     * @param file
     */
    void collectContentFromFile(const QString &file);

    /**
     * @brief collectFromPath
     * @param path
     */
    void collectContentFromPathWithCheck(const QString &path);

    /**
     * @brief contentFromDir
     * @param dir
     */
    void collectContentFromDirWithCheck( const QString &dir);

    /**
     * @brief filesFromDirWithCheck
     * @param dir
     * @return
     */
    QStringList filesFromDirWithCheck(const QString &dir);

    void collectContentFromDir( const QString &dir);

    void collectContentFromPath( const QString &path );

    /**
     * @brief collectRepots
     * @param path
     */
    void collectRepots();

    /**
     * @brief filesFromDir
     * @param dir
     * @return
     */
    QStringList filesFromDir(const QString &dir);

    /**
     * @brief reportsFiles
     * @return
     */
    QStringList reportsFiles();

    /**
     * @brief setRevision
     */
    void setRevision();

    /**
     * @brief checkExistFile
     * @param dir
     * @param file
     * @return
     */
    bool checkExistFile( const QString &dir, const QString &file );

    /**
     * @brief appendInfo
     * @param dirName
     * @param fileName
     */
    void appendInfo(const QString &dirName, const QString &fileName, bool all = false);

    /**
     * @brief appendInfo
     * @param dirName
     * @param file
     */
    void appendInfo(const QString &dirName, const QStringList &file, bool all = false);

    /**
     * @brief copyToDestination
     * @return
     */
    bool copyToDestination();

    /**
     * @brief deleteExcessFiles
     * @return
     */
    bool deleteExcessFiles();

private:
    /// Источник
    QList<SContentInfo>                         mSourceList;

    ///
    QList<SContentInfo>                         mStandartSourceFiles;

    /// Директория назначения
    QString                                     mDest;

    ///
    const CONTENT_TYPE                          mContentType;

    ///
    QString                                     mSourceDir;

    ///
    QMap<CONTENT_TYPE, QString>                 mStringContentType;

    ///
    bool                                        mLocal;

    ///
    QString                                     mBoxId;
};

#endif // CONTENT_H
