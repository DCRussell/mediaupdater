#include "logger.h"
#include "loggerdelete.h"


#include <QTextStream>
#include <QCoreApplication>
#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QTextCodec>

Logger::Logger() :
    mLogFile( new QFile() ),
    mStream(true)
{
    init();
}

Logger::Logger(const QString &text) :
    mLogFile( new QFile() ),
    mStream(false)
{
    init();
    writeLog(text);
}


Logger::Logger(int num) :
    mLogFile( new QFile() ),
    mStream(false)
{
    init();
    writeLog(num);
}


Logger::Logger(uint num ) :
    mLogFile( new QFile() ),
    mStream(false)
{
    init();
    writeLog(num);
}

Logger::~Logger()
{
    if(mStream) {
        writeLog(mText);
    }

    if(mLogFile) {
        if( mLogFile->isOpen() )
            mLogFile->close();
        delete mLogFile;
    }

    LoggerDelete::Instance();
}


void Logger::init()
{
    QString logDir = "logs";
    QString path = QCoreApplication::applicationDirPath() + QDir::separator() + logDir ;

    if( !QDir().mkpath( path ) ) {
        mLogFile->deleteLater();
        mLogFile = nullptr;
        return;
    }

    if( mLogFile )
        mLogFile->setFileName( path + QDir::separator()+ QCoreApplication::applicationName() + QString("_%1.log").arg(QDateTime::currentDateTime().toString("[dd.MM.yyyy]")) );
}


void Logger::writeLog( const QString &text ) const
{
    if( !mLogFile )
        return;
    if(!mLogFile->open(QFile::Append | QFile::Text))
        return;

    QTextStream stream(mLogFile);
    stream << QDateTime::currentDateTime().toString("[dd.MM.yyyy_hh:mm:ss.zzz]: ")
           << text << '\n';
    mLogFile->close();

#ifdef SEND_TO_DEBUG_LOG_MESSAGE
    qDebug() << text;
#endif
}


void Logger::writeLog(int num) const
{
    writeLog( QString::number(num) );
}


void Logger::writeLog(uint num) const
{
    writeLog( QString::number(num));
}


Logger &Logger::operator <<(QString text)
{
    if(!mText.isEmpty())
        mText.append(' ');
    mText.append( text );
    return *this;
}

Logger &Logger::operator <<(int num)
{
    return operator <<(QString::number(num));
}
