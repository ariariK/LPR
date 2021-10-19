#include "thread.h"

Thread::Thread(QObject *parent) :
    QThread(parent)
{
}

Thread::~Thread()
{
    ::qTerminate();
}

void Thread::run()
{
    int i = 0;
    while(1)
    {
        i++;
        emit Send(i);
        msleep(1000);
    }
}
