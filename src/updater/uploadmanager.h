#ifndef UPLOADMANAGER_H
#define UPLOADMANAGER_H

#include "logininfo.h"
#include "content.h"

#include <QObject>
#include <QScopedPointer>

class UploadManager
{
public:
    UploadManager();

    /**
     * @brief start
     */
    void start();

    /**
     * @brief setLoginInfo
     * @param loginInfo
     */
    void setLoginInfo(const LoginInfo &loginInfo);

private:
    ///
    LoginInfo                                   mLoginInfo;

    /**
     * @brief mContent
     */
    QScopedPointer<Content>                     mContent;
};

#endif // UPLOADMANAGER_H
