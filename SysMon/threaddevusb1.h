#ifndef THREADDEVUSB1_H
#define THREADDEVUSB1_H

#include <QThread>
#include <QtDebug>

class ThreadDevUsb1 : public QThread
{
    Q_OBJECT

public:
    explicit ThreadDevUsb1(QObject *parent = 0);
    ~ThreadDevUsb1();

private:

public:

private:
    void run();

signals:
    void Send();
};

#endif // THREADDEVUSB1_H
