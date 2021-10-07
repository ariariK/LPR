#ifndef THREADDEVUSB0_H
#define THREADDEVUSB0_H

#include <QThread>
#include <QtDebug>

class ThreadDevUsb0 : public QThread
{
    Q_OBJECT

public:
    explicit ThreadDevUsb0(QObject *parent = 0);
    ~ThreadDevUsb0();

private:

public:

private:
    void run();

signals:
    void Send();
};

#endif // THREADDEVUSB0_H
