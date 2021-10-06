#include "threadPreview.h"

ThreadPreview::ThreadPreview(QObject *parent) :
    QThread(parent)
{
}

ThreadPreview::~ThreadPreview()
{
    ::qTerminate();
    ExitPreview();
}

void ThreadPreview::InitPreview()
{
}

void ThreadPreview::ExitPreview()
{
}

void ThreadPreview::run(int data)
{
    int i = 0;

#if false
    while(1)
    {
        i++;
        emit Send(i);
        msleep(100);
    }
#endif
}