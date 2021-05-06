#ifndef LOGININFO_H
#define LOGININFO_H


#include <QString>

class LoginInfo {

public:
    LoginInfo();

    LoginInfo(const QString &id, const QString &host, const QString &login, const QString &pass, const QString &prefix);

    void setPort( int port );

    QString host() const;

    QString login() const;

    QString password() const;

    QString boxId() const;

    QString prefixSlash() const;

    QString prefix() const;

    QString scheme() const;

    QString boxFolder() const;
    QString boxFolderSlash() const;

    int     port() const;

    bool isValid();

private:
    QString             mHost;
    QString             mLogin;
    QString             mPass;
    QString             mBoxId;
    QString             mPrefix;
    QString             mScheme;
    QString             mBoxFolder;
    int                 mPort;
};

#endif // LOGININFO_H
