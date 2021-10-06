#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
#ifdef WIN32
    w.show();
#else
    w.showFullScreen();
#endif
    return a.exec();
}
