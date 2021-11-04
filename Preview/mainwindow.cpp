#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFileDialog>
#include <QPixmap>
#include <QDebug>

#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "Spinnaker.h"
#include "SpinGenApi/SpinnakerGenApi.h"

using namespace Spinnaker;
using namespace Spinnaker::GenApi;
using namespace Spinnaker::GenICam;

#define IPC_SM  0
#define IPC_MQ  1
#define IPC_MODE IPC_MQ

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    version = "Ver. Unknown";
    QString ver_file("/etc/.version");
    if (QFile::exists(ver_file) == true) 
    {
	    QFile file(ver_file);
    	if(!file.open(QFile::ReadOnly | QFile::Text))
    	{
    	    	qDebug() << " Could not open file for writing";
    	}
        else 
        {
            // To write text, we use operator<<(),
            // which is overloaded to take
            // a QTextStream on the left
            // and data types (including QString) on the right
            
            QTextStream out(&file);
            version = "Ver. " + out.readAll();
            version.chop(1);
            qDebug() << version;	
            file.flush();
            file.close();
        }
    }
    else
    {
    }

#if true
    setStyleSheet("QWidget#MainWindow { background-color : black; color : white; }");
    setAutoFillBackground( true );
    statusBar()->hide();

    ui->screen->setStyleSheet("QLabel { background-color : black; color : red; }");
    //ui->screen->setStyleSheet("QLabel { background-color : blue; color : white; }");

    // font
    QFont font = ui->screen->font();
    font.setPointSize(20);
    font.setBold(true);
    ui->screen->setFont(font);
#endif
    
    GetParameters();
    
    Sm_Grab = new Ipcs(KEY_NUM_SM, MEM_SIZE_SM);
    Sm_Grab->SharedMemoryInit();
#if (IPC_MODE == IPC_SM)    
    Sm_Res = new Ipcs(KEY_NUM_SM_RES, MEM_SIZE_SM_RES);
    Sm_Res->SharedMemoryInit();
#elif (IPC_MODE == IPC_MQ)
	Mq_Grab	= new Ipcs(KEY_NUM_MQ_GRAB, 0);
    Mq_Grab->MessageQueueInit();
#else
#endif

    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(Update()));
    timer->start(200);

#if false
    threadPreview = new ThreadPreview(this);
    threadPreview->start();
    connect(threadPreview, SIGNAL(Send()), this, SLOT(Run()));
#endif
}

MainWindow::~MainWindow()
{
    disconnect(timer, SIGNAL(timeout()), this, SLOT(Update()));

#if false
    disconnect(threadPreview, SIGNAL(Send()), this, SLOT(Run()));
    delete threadPreview;
#endif

    SAFE_DELETE(Sm_Grab);
#if (IPC_MODE == IPC_SM)    
    SAFE_DELETE(Sm_Res);
#elif (IPC_MODE == IPC_MQ)    
    SAFE_DELETE(Mq_Grab);
#else
#endif
    delete ui;
}

void MainWindow::GetParameters()
{
    capWidth    = 0;
    capHeight   = 0;
}

void MainWindow::Update()
{
    //qDebug() << "Update";

#if (IPC_MODE == IPC_SM)
    Sm_Res->SharedMemoryRead((char *)&st_grab);
    capWidth    = st_grab.capWidth;
    capHeight   = st_grab.capHeight;
    msg.sprintf("%d, %d, %d, %d\n", st_grab.capWidth, st_grab.capHeight, capWidth, capHeight);
    INFO_LOG(msg.toStdString());
#elif (IPC_MODE == IPC_MQ)
    if (!capWidth || !capHeight)
    {
        for(int i=0; i<10; i++)
        {
            if( Mq_Grab->MessageQueueRead((char*)&msq) > 0 )     // non-blocking mode(IPC_NOWAIT)
            {
                capWidth    = msq.data.capWidth;
                capHeight   = msq.data.capHeight;
                break;
            }
            usleep(100000);
            Mq_Grab->MessageQueueInit();
            continue;
        }
        msg.sprintf("%d, %d, %d, %d\n", msq.data.capWidth, msq.data.capHeight, capWidth, capHeight);
        INFO_LOG(msg.toStdString());
    }
#else
#endif

    
    if (capWidth && capHeight)
    {
#if (IPC_MODE == IPC_SM)
#elif (IPC_MODE == IPC_MQ)
        if (Mq_Grab->MessageQueueRead((char*)&msq, 0) < 0)      // blocking mode(0)
        {
            capWidth = 0;
            capHeight = 0;
            fprintf(stderr, "[Error!!!]MessageQueueRead()\n");    
        }
#else
#endif

        // Read Image data
        if (Sm_Grab->SharedMemoryRead((char *)buffer) < 0)
        {
            Sm_Grab->SharedMemoryInit();
            fprintf(stderr, "[Error!!!]SharedMemoryRead()\n");  
        }

        //Mat cvimg = cv::Mat(convertedImage->GetHeight(), convertedImage->GetWidth(), CV_8UC1, convertedImage->GetData(), convertedImage->GetStride());
        Mat cvimg = cv::Mat(capHeight, capWidth, CV_8UC1, (char *)buffer);
        //cv::putText(cvimg, version.toLocal8Bit().data(), cv::Point(msq.data.capWidth - 380, 40), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(255, 0, 0), 4);
        cv::putText(cvimg, version.toLocal8Bit().data(), cv::Point(40, 40), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(255, 0, 0), 4);

        // ���÷��� fix to window
        QPixmap picture = QPixmap::fromImage(QImage((unsigned char*) cvimg.data,
                                        cvimg.cols,
                                        cvimg.rows,
                                        QImage::Format_Grayscale8));
        ui->screen->setPixmap(picture.scaled(ui->screen->size(), Qt::KeepAspectRatio));
    }
    else 
    {
        // Defalut Image Display
        QString qsfileName="/oem/test_data/no_signal.jpg";
        cv::Mat img = cv::imread(qsfileName.toLocal8Bit().data());
        cv::cvtColor(img, img, cv::COLOR_BGR2RGB); // invert BGR to RGB
        cv::putText(img, version.toLocal8Bit().data(), cv::Point(40, 40), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(255, 255, 255), 4);

        QPixmap picture = QPixmap::fromImage(QImage((unsigned char*) img.data,
                                            img.cols,
                                            img.rows,
                                            QImage::Format_RGB888));
        ui->screen->setPixmap(picture.scaled(ui->screen->size(), Qt::KeepAspectRatio));
    }
}

void MainWindow::Run()
{
    qDebug() << "Run";
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

