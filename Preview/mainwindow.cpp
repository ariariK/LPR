#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFileDialog>
#include <QPixmap>
#include <QDebug>

#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

//ImagePtr convertedImage = NULL;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setStyleSheet("QWidget#MainWindow { background-color : black; color : white; }");
    setAutoFillBackground( true );
    statusBar()->hide();


    //ui->screen->setStyleSheet("QLabel { background-color : black; color : white; }");
    ui->screen->setStyleSheet("QLabel { background-color : blue; color : white; }");
    ui->screen->setContentsMargins(0,0,0,0);
    ui->screen->setMinimumWidth( this->width() );
    ui->screen->setMinimumHeight( this->height() );

    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(Update()));
    timer->start(10);

    threadPreview = new ThreadPreview(this);
    threadPreview->start();
    connect(threadPreview, SIGNAL(Send(int)), this, SLOT(Run(int)));
}

MainWindow::~MainWindow()
{
    disconnect(timer, SIGNAL(timeout()), this, SLOT(Update()));
    disconnect(threadPreview, SIGNAL(Send(int)), this, SLOT(Run(int)));
    delete threadPreview;
    delete ui;
}

void MainWindow::Update()
{
    qDebug() << "Update";

    ui->screen->setContentsMargins(0,0,0,0);
    ui->screen->setMinimumWidth( this->width() );
    ui->screen->setMinimumHeight( this->height() );

#if false    
    if (convertedImage != NULL) {
/*
    unsigned int XPadding = convertedImage->GetXPadding();
    unsigned int YPadding = convertedImage->GetYPadding();
    unsigned int rowsize = convertedImage->GetWidth();
    unsigned int colsize = convertedImage->GetHeight();
*/

        Mat cvimg = cv::Mat(convertedImage->GetHeight(), convertedImage->GetWidth(), CV_8UC1, convertedImage->GetData(), convertedImage->GetStride());

        //cv::cvtColor(cvimg, cvimg, cv::COLOR_BGR2RGB); // invert BGR to RGB

        // ���÷��� fix to window
        QPixmap picture = QPixmap::fromImage(QImage((unsigned char*) cvimg.data,
                                        cvimg.cols,
                                        cvimg.rows,
                                        QImage::Format_Grayscale8));
        ui->screen->setPixmap(picture.scaled(ui->screen->size(), Qt::KeepAspectRatio));
    }
#endif
}

void MainWindow::Run(int data)
{
    qDebug() << "Run(data)";
}

#if false
void MainWindow::on_pushButton_clicked()
{
    QString qsfileName = QFileDialog::getOpenFileName(this,
                                                      tr("Open Image"),
                                                      "../",
                                                      tr("Image Files (*.png *.jpg *.bmp)")
                                                      );
    // �ѱ� ����(toLocal8Bit())
    qDebug() << "select file name : " << qsfileName.toLocal8Bit().data();

    if (!qsfileName.isEmpty())
    {
        //cv::Mat img = cv::imread(qsfileName.toStdString());
        // �ѱ� ����(toLocal8Bit())
        cv::Mat img = cv::imread(qsfileName.toLocal8Bit().data());

        //cv::imshow("Hello OpenCV", img);
        // �÷� �����̽� ���� (imread(bgr) to RGB)
        cv::cvtColor(img, img, cv::COLOR_BGR2RGB); // invert BGR to RGB

        // ���÷��� fix to window
        QPixmap picture = QPixmap::fromImage(QImage((unsigned char*) img.data,
                                              img.cols,
                                              img.rows,
                                              QImage::Format_RGB888));
        ui->screen->setPixmap(picture.scaled(ui->screen->size(), Qt::KeepAspectRatio));
        //QImage  src(img.data, img.cols, img.rows, img.step, QImage::Format_RGB888);
        //ui->screen->setPixmap(QPixmap::fromImage(src));
    }
}
#endif

