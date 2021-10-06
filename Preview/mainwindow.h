#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "threadPreview.h"

#include <QMainWindow>
#include <QTimer>
#include <QLabel>

#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/core/utils/trace.hpp>

using namespace cv;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    QLabel *screen;
    

private slots:
    void Update();
    void Run(int data);

private:
    QTimer *timer;

    Ui::MainWindow *ui;
    ThreadPreview *threadPreview;
};

#endif // MAINWINDOW_H
