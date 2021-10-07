#include <QProcess>

#include <iostream>
#include <sstream>
#include <vector>

#include "mainwindow.h"
#include "ui_mainwindow.h"

using namespace std;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Set Title
    this->setWindowTitle("LPR - AI Edge Device");

    // QProgressBar
    value = 0;

    // result of ethernet port 0
    ui->label_eth0->setStyleSheet("QLabel { background-color : gray; color : black; }");
    ui->label_eth0->setText("ready...");
    ui->label_eth0_conn->setStyleSheet("QLabel { background-color : gray; color : white; }");
    ui->label_eth0_conn->setText("ready...");
    //ui->label_eth0_conn->hide();

    // result of ethernet port 1
    ui->label_eth1->setStyleSheet("QLabel { background-color : gray; color : black; }");
    ui->label_eth1->setText("ready...");
    ui->label_eth1_conn->setStyleSheet("QLabel { background-color : gray; color : white; }");
    ui->label_eth1_conn->setText("ready...");
    //ui->label_eth1_conn->hide();

    // result of usb port 1 (front 2.0)
    ui->label_usb_f2->setStyleSheet("QLabel { background-color : gray; color : black; }");
    ui->label_usb_f2->setText("ready...");
    ui->label_usb_f2_conn->setStyleSheet("QLabel { background-color : gray; color : white; }");
    ui->label_usb_f2_conn->setText("ready...");
    //ui->label_usb_f2_conn->hide();

    // result of usb port 2 (front 3.0)
    ui->label_usb_f3->setStyleSheet("QLabel { background-color : gray; color : black; }");
    ui->label_usb_f3->setText("ready...");
    ui->label_usb_f3_conn->setStyleSheet("QLabel { background-color : gray; color : white; }");
    ui->label_usb_f3_conn->setText("ready...");
    //ui->label_usb_f3_conn->hide();

    // result of usb port 3 (rear 3.0)
    ui->label_usb_r3->setStyleSheet("QLabel { background-color : gray; color : black; }");
    ui->label_usb_r3->setText("ready...");
    ui->label_usb_r3_conn->setStyleSheet("QLabel { background-color : gray; color : white; }");
    ui->label_usb_r3_conn->setText("ready...");
    //ui->label_usb_r3_conn->hide();


    // 쓰레드
    thread = new Thread(this);
    thread->start();
    connect(thread, SIGNAL(Send(int)), this, SLOT(Receive(int)));

    // thread - dev ethernet0
    thDevEth0 = new ThreadDevEth0(this);
    thDevEth0->start();
    connect(thDevEth0, SIGNAL(Send()), this, SLOT(updateEth0()));

    // thread - dev ethernet1
    thDevEth1 = new ThreadDevEth1(this);
    thDevEth1->start();
    connect(thDevEth1, SIGNAL(Send()), this, SLOT(updateEth1()));

    // thread - dev usb0
    thDevUsb0 = new ThreadDevUsb0(this);
    thDevUsb0->start();
    connect(thDevUsb0, SIGNAL(Send()), this, SLOT(updateUsb0()));

    // thread - dev usb1
    thDevUsb1 = new ThreadDevUsb1(this);
    thDevUsb1->start();
    connect(thDevUsb1, SIGNAL(Send()), this, SLOT(updateUsb1()));

    // thread - dev usb2
    thDevUsb2 = new ThreadDevUsb2(this);
    thDevUsb2->start();
    connect(thDevUsb2, SIGNAL(Send()), this, SLOT(updateUsb2()));
}

MainWindow::~MainWindow()
{
    delete thread;
    delete thDevEth0;
    delete ui;
}

void MainWindow::Receive(int data)
{
    //qDebug() << data;
    updateProgressBar();
}

void MainWindow::updateEth0()
{
    QProcess pEth0;
    pEth0.start("ifconfig eth0");
    pEth0.waitForFinished();
    QString output(pEth0.readAllStandardOutput());
    //qDebug() << output;
    cout << output.toStdString() << endl;

    // 이더넷 디바이스 체크
    if (output.isEmpty() || output.contains("Device not found", Qt::CaseInsensitive))   // not found
    {
        ui->label_eth0->setStyleSheet("QLabel { background-color : red; color : white; }");
        ui->label_eth0->setText("not found");
    }
    // 상태 정보 전송
    else
    {
        ui->label_eth0->setStyleSheet("QLabel { background-color : green; color : white; }");
        ui->label_eth0->setText("OK");
    }

    // 종료
    pEth0.close();
}

void MainWindow::updateEth1()
{
    QProcess pEth1;
    pEth1.start("ifconfig eth1");
    pEth1.waitForFinished();
    QString output(pEth1.readAllStandardOutput());
    //qDebug() << output;
    cout << output.toStdString() << endl;

    // 이더넷 디바이스 체크
    if (output.isEmpty() || output.contains("Device not found", Qt::CaseInsensitive))   // not found
    {
        ui->label_eth1->setStyleSheet("QLabel { background-color : red; color : white; }");
        ui->label_eth1->setText("not found");
    }
    // 상태 정보 전송
    else
    {
        ui->label_eth1->setStyleSheet("QLabel { background-color : green; color : white; }");
        ui->label_eth1->setText("OK");
    }

    // 종료
    pEth1.close();
}

// Front 2.0 (Bus 0 and 1)
void MainWindow::updateUsb0()
{
    QProcess pUsb0;
    pUsb0.start("sh");
    //pUsb0.write("lsusb | grep \"Bus 001\\\|Bus 002\"");
    pUsb0.write("lsusb | grep \"Bus 003\\\|Bus 004\"");
    pUsb0.closeWriteChannel();
    pUsb0.waitForFinished();
    QString output(pUsb0.readAllStandardOutput());
    //qDebug() << output;
    cout << output.toStdString() << endl;

    // parse
    string token;
    stringstream ss(output.toStdString());
    vector<string> buf;
    while(getline(ss, token, '\n'))     // 라인단위 분리
    {
        buf.push_back(token);
    };

    int vcount = buf.size();
    int tree_info_cnt = 0;      // USB 디바이스 정보
    int dev_info_cnt  = 0;      // USB 연결정보
    string dev_id = "";
    for (int i=0; i<vcount; i++)
    {
        dev_id = buf[i].substr(buf[i].find("ID ")+3, 4);
        if (dev_id.compare("1d6b") == 0)
        {
            tree_info_cnt++;
        }
        else
        {
            dev_info_cnt = 1;
        }
    }

    // USB 디바이스 드라이버 정보
    if (tree_info_cnt < 2)
    {
        ui->label_usb_f2->setStyleSheet("QLabel { background-color : red; color : white; }");
        ui->label_usb_f2->setText("not found");
    }
    else if (tree_info_cnt == 2)
    {
        ui->label_usb_f2->setStyleSheet("QLabel { background-color : green; color : white; }");
        ui->label_usb_f2->setText("OK");
    }


    // USB 연결정보
    if (dev_info_cnt == 1)
    {
        //ui->label_usb_f2_conn->show();
        ui->label_usb_f2_conn->setStyleSheet("QLabel { background-color : green; color : white; }");
        ui->label_usb_f2_conn->setText("Connected");
    }
    else
    {
        //ui->label_usb_f2_conn->hide();
        ui->label_usb_f2_conn->setStyleSheet("QLabel { background-color : gray; color : white; }");
        ui->label_usb_f2_conn->setText("ready...");
    }
}

// Front 3.0 (Bus 7 and 8)
void MainWindow::updateUsb1()
{
    QProcess pUsb1;
    pUsb1.start("sh");
    pUsb1.write("lsusb | grep \"Bus 007\\\|Bus 008\"");
    pUsb1.closeWriteChannel();
    pUsb1.waitForFinished();
    QString output(pUsb1.readAllStandardOutput());
    //qDebug() << output;
    cout << output.toStdString() << endl;

    // parse
    string token;
    stringstream ss(output.toStdString());
    vector<string> buf;
    while(getline(ss, token, '\n'))     // 라인단위 분리
    {
        buf.push_back(token);
    };

    int vcount = buf.size();
    int tree_info_cnt = 0;      // USB 디바이스 정보
    int dev_info_cnt  = 0;      // USB 연결정보
    string dev_id = "";
    for (int i=0; i<vcount; i++)
    {
        dev_id = buf[i].substr(buf[i].find("ID ")+3, 4);
        if (dev_id.compare("1d6b") == 0)
        {
            tree_info_cnt++;
        }
        else
        {
            dev_info_cnt = 1;
        }
    }

    // USB 디바이스 드라이버 정보
    if (tree_info_cnt < 2)
    {
        ui->label_usb_f3->setStyleSheet("QLabel { background-color : red; color : white; }");
        ui->label_usb_f3->setText("not found");
    }
    else if (tree_info_cnt == 2)
    {
        ui->label_usb_f3->setStyleSheet("QLabel { background-color : green; color : white; }");
        ui->label_usb_f3->setText("OK");
    }


    // USB 연결정보
    if (dev_info_cnt == 1)
    {
        //ui->label_usb_f3_conn->show();
        ui->label_usb_f3_conn->setStyleSheet("QLabel { background-color : green; color : white; }");
        ui->label_usb_f3_conn->setText("Connected");
    }
    else
    {
        //ui->label_usb_f3_conn->hide();
        ui->label_usb_f3_conn->setStyleSheet("QLabel { background-color : gray; color : white; }");
        ui->label_usb_f3_conn->setText("ready...");
    }
}

// Rear 3.0 (Bus 5 and 6)
void MainWindow::updateUsb2()
{
    QProcess pUsb2;
    pUsb2.start("sh");
    pUsb2.write("lsusb | grep \"Bus 005\\\|Bus 006\"");
    pUsb2.closeWriteChannel();
    pUsb2.waitForFinished();
    QString output(pUsb2.readAllStandardOutput());
    //qDebug() << output;
    cout << output.toStdString() << endl;

    // parse
    string token;
    stringstream ss(output.toStdString());
    vector<string> buf;
    while(getline(ss, token, '\n'))     // 라인단위 분리
    {
        buf.push_back(token);
    };

    int vcount = buf.size();
    int tree_info_cnt = 0;      // USB 디바이스 정보
    int dev_info_cnt  = 0;      // USB 연결정보
    string dev_id = "";
    for (int i=0; i<vcount; i++)
    {
        dev_id = buf[i].substr(buf[i].find("ID ")+3, 4);
        if (dev_id.compare("1d6b") == 0)
        {
            tree_info_cnt++;
        }
        else
        {
            dev_info_cnt = 1;
        }
    }

    // USB 디바이스 드라이버 정보
    if (tree_info_cnt < 2)
    {
        ui->label_usb_r3->setStyleSheet("QLabel { background-color : red; color : white; }");
        ui->label_usb_r3->setText("not found");
    }
    else if (tree_info_cnt == 2)
    {
        ui->label_usb_r3->setStyleSheet("QLabel { background-color : green; color : white; }");
        ui->label_usb_r3->setText("OK");
    }


    // USB 연결정보
    if (dev_info_cnt == 1)
    {
        //ui->label_usb_r3_conn->show();
        ui->label_usb_r3_conn->setStyleSheet("QLabel { background-color : green; color : white; }");
        ui->label_usb_r3_conn->setText("Connected");
    }
    else
    {
        //ui->label_usb_r3_conn->hide();
        ui->label_usb_r3_conn->setStyleSheet("QLabel { background-color : gray; color : white; }");
        ui->label_usb_r3_conn->setText("ready...");
    }
}

void MainWindow::updateProgressBar()
{
    value++;
    ui->progressBar->setValue(value%100);
}
