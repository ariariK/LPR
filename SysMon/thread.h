#ifndef THREAD_H
#define THREAD_H

#include <QThread>
#include <QtDebug>

class Thread : public QThread
{
    Q_OBJECT

public:
    explicit Thread(QObject *parent = 0);
    ~Thread();

private:
    void run();

signals:
    void Send(int data);
};

#endif // THREAD_H
