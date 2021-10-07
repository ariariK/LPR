#include "threaddeveth1.h"

ThreadDevEth1::ThreadDevEth1(QObject *parent) :
    QThread(parent)
{

}

ThreadDevEth1::~ThreadDevEth1()
{
    ::qTerminate();
}

void ThreadDevEth1::run()
{
    while(1)
    {
        emit Send();

        sleep(1);
    }
}
