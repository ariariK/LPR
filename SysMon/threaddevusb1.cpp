#include "threaddevusb1.h"

ThreadDevUsb1::ThreadDevUsb1(QObject *parent) :
    QThread(parent)
{

}

ThreadDevUsb1::~ThreadDevUsb1()
{
    ::qTerminate();
}

void ThreadDevUsb1::run()
{
    while(1)
    {
        emit Send();

        sleep(1);
    }
}
