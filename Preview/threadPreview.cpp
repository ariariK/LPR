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

void ThreadPreview::run()
{
#if true
    while(1)
    {
        emit Send();
        msleep(100);
        break;
    }
#endif
}