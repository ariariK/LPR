#ifndef THREADDEVGPIO_H
#define THREADDEVGPIO_H

#include <QThread>
#include <QtDebug>

class ThreadDevGpio : public QThread
{
    Q_OBJECT

public:
    explicit ThreadDevGpio(QObject *parent = 0);
    ~ThreadDevGpio();

private:

public:

private:
    void run();

signals:
    void Send();

};

#endif // THREADDEVGPIO_H
