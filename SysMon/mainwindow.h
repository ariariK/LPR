#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "thread.h"         // for Thread
#include "threaddeveth0.h"
#include "threaddeveth1.h"
#include "threaddevusb0.h"
#include "threaddevusb1.h"
#include "threaddevusb2.h"

#include <QMainWindow>
#include <QProgressBar>
#include <QTime>

#include <time.h>
#include <syslog.h>
#define EMERG_LOG(fmt, ...)     { syslog(LOG_EMERG,   "[Emergency][%s: %d] %s", __FILE__, __LINE__, fmt.c_str()); }
#define ALERT_LOG(fmt, ...)     { syslog(LOG_ALERT,   "[Alert][%s: %d] %s", __FILE__, __LINE__, fmt.c_str()); }
#define CRIT_LOG(fmt, ...)      { syslog(LOG_CRIT,    "[Critical][%s: %d] %s", __FILE__, __LINE__, fmt.c_str()); }
#define ERR_LOG(fmt, ...)       { syslog(LOG_ERR,     "[Error][%s: %d] %s", __FILE__, __LINE__, fmt.c_str()); }
#define WARN_LOG(fmt, ...)      { syslog(LOG_WARNING, "[Warning][%s: %d] %s", __FILE__, __LINE__, fmt.c_str()); }
#define NOTICE_LOG(fmt, ...)    { syslog(LOG_NOTICE,  "[Notice][%s: %d] %s", __FILE__, __LINE__, fmt.c_str()); }
#define INFO_LOG(fmt, ...)      { syslog(LOG_INFO,    "[Info][%s: %d] %s", __FILE__, __LINE__, fmt.c_str()); }
#define DEBUG_LOG(fmt, ...)     { syslog(LOG_DEBUG,   "[Debug][%s: %d] %s", __FILE__, __LINE__, fmt.c_str()); }


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    qint64 value;
    QProgressBar *progressBar;

    int fd[2];
    pid_t childpid;

    QTime runTime;
    time_t startTime;


private:
    Ui::MainWindow *ui;
    Thread *thread;
    ThreadDevEth0   *thDevEth0;
    ThreadDevEth1   *thDevEth1;
    ThreadDevUsb0   *thDevUsb0;
    ThreadDevUsb1   *thDevUsb1;
    ThreadDevUsb2   *thDevUsb2;

    void updateMacAddress();
    void updateProgressBar();

    // IPC
    void initPIPE();
    void readPIPE();
    void writePIPE();
    void closePIPE();

private slots:
    void Run(int data);

    void updateEth0();
    void updateEth1();
    void updateUsb0();
    void updateUsb1();
    void updateUsb2();

};
#endif // MAINWINDOW_H
