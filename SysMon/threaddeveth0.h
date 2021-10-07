#ifndef THREADDEVETH0_H
#define THREADDEVETH0_H

#include <QThread>
#include <QtDebug>

class ThreadDevEth0 : public QThread
{
    Q_OBJECT

public:
    explicit ThreadDevEth0(QObject *parent = 0);
    ~ThreadDevEth0();

private:

public:

private:
    void run();

signals:
    void Send();

};

#endif // THREADDEVETH0_H
