#include "threaddevusb0.h"

ThreadDevUsb0::ThreadDevUsb0(QObject *parent) :
    QThread(parent)
{

}

ThreadDevUsb0::~ThreadDevUsb0()
{
    ::qTerminate();
}

void ThreadDevUsb0::run()
{
    while(1)
    {
        emit Send();

        sleep(1);
    }
}
