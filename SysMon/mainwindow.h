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

private:
    Ui::MainWindow *ui;
    Thread *thread;
    ThreadDevEth0   *thDevEth0;
    ThreadDevEth1   *thDevEth1;
    ThreadDevUsb0   *thDevUsb0;
    ThreadDevUsb1   *thDevUsb1;
    ThreadDevUsb2   *thDevUsb2;

    void updateProgressBar();

private slots:
    void Receive(int data);

    void updateEth0();
    void updateEth1();
    void updateUsb0();
    void updateUsb1();
    void updateUsb2();

};
#endif // MAINWINDOW_H
