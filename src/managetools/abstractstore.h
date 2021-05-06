#ifndef O2ABSTRACTSTORE_H
#define O2ABSTRACTSTORE_H

#include <QObject>
#include <QString>

class AbstractStore: public QObject
{
    Q_OBJECT

public:

    explicit AbstractStore(QObject *parent = 0): QObject(parent) {
    }

    virtual ~AbstractStore() {
    }

    virtual QString value(const QString &key, const QString &defaultValue = QString()) = 0;

    virtual void setValue(const QString &key, const QString &value) = 0;
};

#endif // O2ABSTRACTSTORE_H
