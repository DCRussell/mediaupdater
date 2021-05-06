#ifndef FILEUPLOAD_H
#define FILEUPLOAD_H

#include <QObject>

class FileUploader : public QObject
{
    Q_OBJECT
public:
    explicit FileUploader(QObject *parent = 0);

signals:

public slots:
};

#endif // FILEUPLOAD_H
