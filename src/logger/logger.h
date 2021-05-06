#ifndef _LOGGER_H_
#define _LOGGER_H_

#include <QFile>

class Logger
{
public:
    Logger();
    explicit Logger( const QString &text);
    explicit Logger( int num);
    explicit Logger( uint num );

    Logger &operator <<(QString text);
    Logger &operator <<(int num);

    ~Logger();

private:
    void writeLog( const QString &text ) const;
    void writeLog( int num ) const;
    void writeLog( uint num ) const;

    void init();


private:
    QFile                                  * mLogFile;
    QString                                 mText;
    bool                                    mStream;

};

#endif // _LOGGER_H_
