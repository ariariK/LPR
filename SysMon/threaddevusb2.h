#ifndef THREADDEVUSB2_H
#define THREADDEVUSB2_H

#include <QThread>
#include <QtDebug>

class ThreadDevUsb2 : public QThread
{
    Q_OBJECT

public:
    explicit ThreadDevUsb2(QObject *parent = 0);
    ~ThreadDevUsb2();

private:

public:

private:
    void run();

signals:
    void Send();
};

#endif // THREADDEVUSB2_H
