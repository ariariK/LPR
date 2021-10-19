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

///////////////////////////////////////////////////////////////////////////////////////////
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <errno.h>

#define  KEY_NUM_SM		1234
#define  MEM_SIZE_SM	512*4096

#define  KEY_NUM_MQ     2345


char buffer[MEM_SIZE_SM] = {0,};
int MainWindow::SharedMemoryInit(void)
{
    if((shmid = shmget((key_t)KEY_NUM_SM, 0, 0)) == -1)
    {
        perror("Shmid failed");

        return -1;
    }

    return 1;
}

int MainWindow::SharedMemoryRead(char *sMemory)
{
    void *shmaddr;
    
    if((shmaddr = shmat(shmid, (void *)0, 0)) == (void *)-1)
    {
        perror("Shmat failed");
        return -1;
    }

    memcpy(sMemory, (char *)shmaddr, MEM_SIZE_SM);
    
    if(shmdt(shmaddr) == -1)
    {
        perror("Shmdt failed");
        return -1;
    }
    
    return 0;
}

int MainWindow::MessageQueueInit()
{
    //받아오는 쪽의 msqid얻어오고
    if ((msqid=msgget((key_t)KEY_NUM_MQ, IPC_CREAT|0666)) == -1)
    {
        perror("msgget failed\n");
        return -1;
    }

    return 1;
}

int MainWindow::MessageQueueRead()
{
    //if(msgrcv(msqid, &msq, sizeof(struct real_data), 0, 0)==-1){
    if (msgrcv(msqid, &msq, sizeof(struct real_data), 0, IPC_NOWAIT) == -1)
    {
        if (errno != ENOMSG)
        {
            printf("msgrcv failed : %d\n", errno);
            return -1;
        }
        return 1;
    }
    //printf("name : %d, age :%d\n", msq.data.capWidth, msq.data.capHeight);

    return 1;
}

int MainWindow::MessageQueueFree()
{
    if (msgctl(msqid, IPC_RMID, NULL)==-1)
    {
        perror("MessageQueueFree() : msgctl failed\n");
        return -1;
    }

	return 1;
}

///////////////////////////////////////////////////////////////////////////////////////////

//ImagePtr convertedImage = NULL;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

#if true
    setStyleSheet("QWidget#MainWindow { background-color : black; color : white; }");
    setAutoFillBackground( true );
    statusBar()->hide();

    ui->screen->setStyleSheet("QLabel { background-color : black; color : white; }");
    //ui->screen->setStyleSheet("QLabel { background-color : blue; color : white; }");
#endif

    GetParameters();
    SharedMemoryInit();
    MessageQueueInit();

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

    delete ui;
}

void MainWindow::GetParameters()
{
}

void MainWindow::Update()
{
    //qDebug() << "Update";

#if true    

    MessageQueueRead();
    SharedMemoryRead((char *)buffer);

    //Mat cvimg = cv::Mat(convertedImage->GetHeight(), convertedImage->GetWidth(), CV_8UC1, convertedImage->GetData(), convertedImage->GetStride());
    Mat cvimg = cv::Mat(msq.data.capHeight, msq.data.capWidth, CV_8UC1, (char *)buffer);

    //cv::cvtColor(cvimg, cvimg, cv::COLOR_BGR2RGB); // invert BGR to RGB

    // ���÷��� fix to window
    QPixmap picture = QPixmap::fromImage(QImage((unsigned char*) cvimg.data,
                                    cvimg.cols,
                                    cvimg.rows,
                                    QImage::Format_Grayscale8));
    if(picture.isNull())
    {
        QString qsfileName="/oem/test_data/no_signal.jpg";
        cv::Mat img = cv::imread(qsfileName.toLocal8Bit().data());
        cv::cvtColor(img, img, cv::COLOR_BGR2RGB); // invert BGR to RGB

        QPixmap picture = QPixmap::fromImage(QImage((unsigned char*) img.data,
                                              img.cols,
                                              img.rows,
                                              QImage::Format_RGB888));
        ui->screen->setPixmap(picture.scaled(ui->screen->size(), Qt::KeepAspectRatio));
    }
    else 
    {
        ui->screen->setPixmap(picture.scaled(ui->screen->size(), Qt::KeepAspectRatio));
    }

#endif
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

