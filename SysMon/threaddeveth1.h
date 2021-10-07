#ifndef THREADDEVETH1_H
#define THREADDEVETH1_H

#include <QThread>
#include <QtDebug>

class ThreadDevEth1 : public QThread
{
    Q_OBJECT

public:
    explicit ThreadDevEth1(QObject *parent = 0);
    ~ThreadDevEth1();

private:

public:

private:
    void run();

signals:
    void Send();
};

#endif // THREADDEVETH1_H
