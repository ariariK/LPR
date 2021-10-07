#include "threaddevusb2.h"

ThreadDevUsb2::ThreadDevUsb2(QObject *parent) :
    QThread(parent)
{

}

ThreadDevUsb2::~ThreadDevUsb2()
{
    ::qTerminate();
}

void ThreadDevUsb2::run()
{
    while(1)
    {
        emit Send();

        sleep(1);
    }
}
