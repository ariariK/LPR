#include "threaddevgpio.h"

ThreadDevGpio::ThreadDevGpio(QObject *parent) :
    QThread(parent)
{

}

ThreadDevGpio::~ThreadDevGpio()
{
    ::qTerminate();
}

void ThreadDevGpio::run()
{
    while(1)
    {
        emit Send();

        //sleep(1);
        msleep(200);
    }
}
