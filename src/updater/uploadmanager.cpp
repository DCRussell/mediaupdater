#include "uploadmanager.h"


UploadManager::UploadManager()
{

}

void UploadManager::start()
{
    if( !mLoginInfo.isValid() )
        return;

    mContent.reset( new Content(REPORT, mLoginInfo.boxId() ) );
}


void UploadManager::setLoginInfo(const LoginInfo &loginInfo)
{
    mLoginInfo = loginInfo;
}
