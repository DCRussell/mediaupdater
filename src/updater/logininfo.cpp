#include "logininfo.h"


LoginInfo::LoginInfo() {
    //
}

LoginInfo::LoginInfo(const QString &id, const QString &host, const QString &login, const QString &pass, const QString &prefix) :
    mHost(host),
    mLogin( login ),
    mPass( pass ),
    mBoxId( id ),
    mPrefix( prefix )

{
    mScheme     = "ftp";
    mBoxFolder  = "mediabox";
    mPort       = 21;
}

void LoginInfo::setPort( int port ) {
    mPort = port;
}

QString LoginInfo::host() const {
    return mHost;
}

QString LoginInfo::login() const {
    return mLogin;
}

QString LoginInfo::password() const {
    return mPass;
}

QString LoginInfo::boxId() const {
    return mBoxId;
}

QString LoginInfo::prefixSlash() const {
    if( mPrefix.isEmpty() )
        return mPrefix;

    if(mPrefix.right(1) != "/")
        return mPrefix + "/";
    else
        return mPrefix;
}

QString LoginInfo::prefix() const {
    return mPrefix;
}

QString LoginInfo::scheme() const {
    return mScheme;
}

QString LoginInfo::boxFolder() const
{
    return mBoxFolder;
}

QString LoginInfo::boxFolderSlash() const
{
    if( mBoxFolder.isEmpty() )
        return mBoxFolder;

    if(mBoxFolder.right(1) != "/")
        return mBoxFolder + "/";
    else
        return mBoxFolder;
}

int LoginInfo::port() const
{
    return mPort;
}

bool LoginInfo::isValid()
{
    return !mHost.isEmpty() && !mLogin.isEmpty() && !mPass.isEmpty() && !mBoxId.isEmpty();
}
