#include "threaddeveth0.h"

ThreadDevEth0::ThreadDevEth0(QObject *parent) :
    QThread(parent)
{

}

ThreadDevEth0::~ThreadDevEth0()
{
    ::qTerminate();
}

void ThreadDevEth0::run()
{
    while(1)
    {
        emit Send();

        sleep(1);
    }
}
