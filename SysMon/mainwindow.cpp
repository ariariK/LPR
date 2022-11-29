#include <QProcess>
#include <QFile>
#include <QTextStream>
#include <QDesktopWidget>

#include <iostream>
#include <sstream>
#include <vector>

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>

using namespace std;

// add. by ariar : 2022.07.18 - begin
#define FD_LED_POWER	"/sys/class/gpio/gpio91/value"
#define FD_LED_SYSTEM	"/sys/class/gpio/gpio157/value"
#define FD_LED_CAMERA	"/sys/class/gpio/gpio158/value"
#define FD_LED_SWITCH	"/sys/class/gpio/gpio52/value"	// IR STATUS(Enable output)
// add. by ariar : 2022.07.18 - end

// add. by ariar : 2022.10.12 - begin
#define MARGIN_HOR 18
#define MARGIN_VER 18
// add. by ariar : 2022.10.12 - end

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{

    ui->setupUi(this);

    // 동작시간
    //runTime.start();
    startTime = time(NULL); // 현재 시간을 받음

    // Set Title
    this->setWindowTitle("LPR - AI Edge Device");

    // Init
    stResult = {0,};
    stResult.ST_Eth0 = 0;
    stResult.ST_Eth1 = 0;
    stResult.ST_USB_F2_0 = 0;
    stResult.ST_USB_F3_0 = 0;

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
    ui->label_4->hide();                // add. by ariari : 2022.07.18
    ui->label_usb_r3->hide();           // add. by ariari : 2022.07.18
    ui->label_usb_r3_conn->hide();      // add. by ariari : 2022.07.18

    // MAC Address
    ui->label_SW->setStyleSheet("QLabel { background-color : gray; color : black; }");
    ui->label_mac->setText("ready...");
    ui->label_mac->hide();                // add. by ariari : 2022.10.12
    ui->label_10->hide();                // add. by ariari : 2022.10.12

    // add. by ariar : 2022.07.18 - begin
    ui->label_SW->setStyleSheet("QLabel { background-color : gray; color : black; }");
    ui->label_CAM->setStyleSheet("QLabel { background-color : gray; color : black; }");
    ui->label_SYS->setStyleSheet("QLabel { background-color : gray; color : black; }");
    ui->label_PWR->setStyleSheet("QLabel { background-color : gray; color : black; }");

    ui->label_13->setStyleSheet("QLabel { background-color : red; color : black; }");
    ui->label_13->setText("NG");
    // add. by ariar : 2022.07.18 - end

    // IPC
    initPIPE();

    // 쓰레드
    thread = new Thread(this);
    thread->start();
    connect(thread, SIGNAL(Send(int)), this, SLOT(Run(int)));

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

    // add. by ariar : 2022.07.18 - begin
    thDevGpio = new ThreadDevGpio(this);
    thDevGpio->start();
    connect(thDevGpio, SIGNAL(Send()), this, SLOT(updateGPIOs()));
    // add. by ariar : 2022.07.18 - end

#ifdef WIN32
#else
    //QString version("Ver. Unknown");
    stResult.fw_version = "Ver. Unknown";
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
            stResult.fw_version = "Ver. " + out.readAll();
            qDebug() << stResult.fw_version;	
            file.flush();
            file.close();
        }
    }
    else
    {
    }
    ui->label_version->setText(stResult.fw_version);
#endif
}

MainWindow::~MainWindow()
{
    delete thread;
    delete thDevEth0;
    delete thDevEth1;
    delete thDevUsb0;
    delete thDevUsb1;
    delete thDevUsb2;
    delete thDevGpio;

    closePIPE();

    delete ui;
}

void MainWindow::CheckScreenGeometry()
{
    // check connection 
    // /sys/class/drm/card0-HDMI-A-1/status : connected or disconnected
    QFile file("/sys/class/drm/card0-HDMI-A-1/status");
    if(!file.open(QFile::ReadOnly | QFile::Text))
    {
        qDebug() << " Could not open the file for reading";
        //return "";
    }

    QTextStream in(&file);
    QString statusTxt = in.readLine();
    file.close();

    // HDMI disconnected
    if(statusTxt.compare("disconnected") == 0) return;

    // add. by ariari : 2022.11.11 - begin
    QFile file_mode("/sys/class/drm/card0-HDMI-A-1/mode");
    if(!file_mode.open(QFile::ReadOnly | QFile::Text))
    {
        qDebug() << " Could not open the file for reading";
        //return "";
    }

    QTextStream in_mode(&file_mode);
    QString modeTxt = in_mode.readLine();
    file_mode.close();
    QStringList list = modeTxt.split("x");
    screenWidth = stoi(list.at(0).toStdString().c_str());
    screenHeight = stoi(list.at(1).toStdString().c_str());
    //printf("HDMI Mode = %s, (%dx%d)\n", modeTxt.toStdString().c_str(), screenWidth, screenHeight);
    // add. by ariari : 2022.11.11 - end

    QMainWindow *window = new QMainWindow();
    QRect rec = QApplication::desktop()->screenGeometry();
    if (screenWidth != rec.width() || screenHeight != rec.height())
    {
        //printf("screenWidth : %d, screenHeight : %d, rec.width() : %d, rec.height() : %d\n", screenWidth, screenHeight, rec.width(), rec.height());

        screenWidth     = rec.width();
        screenHeight    = rec.height();
        window->move(QPoint(rec.x(), rec.y()));
        window->resize(rec.width(), rec.height());
    }
    delete window;
}

void MainWindow::updateProgressBar()
{
    CheckScreenGeometry();  // add. by ariari : 2022.10.12

    // Running Time
#if false
    int nElapsedTime = runTime.elapsed();

    value++;
    //ui->progressBar->setValue(value%100);
    ui->progressBar->setTextVisible(true);
    ui->progressBar->setFormat(QTime::fromMSecsSinceStartOfDay(nElapsedTime).toString("hh:mm:ss"));
#else
    time_t curTime;

    curTime = time(NULL);       // 현재 시간을 받음
    double diff_timer = difftime(curTime, startTime);


    int day = (int)(diff_timer/60)/60/24;
    int hour = (int)((diff_timer/60)/60)%24;
    int min = (int)(diff_timer/60)%60;
    int sec = (int)diff_timer%60;

    QString strElapsedTime;
    strElapsedTime.sprintf("%02d:%02d:%02d:%02d", day, hour, min, sec);
    ui->progressBar->setFormat(strElapsedTime);

    INFO_LOG(strElapsedTime.toStdString());
#endif

    // 검사결과 저장(/oem/report_sysmon.txt)
    QFile result("/oem/tmp_report.txt");
    //result.open(QFile::WriteOnly|QFile::Append|QFile::Text)); // 쓰기 전용, 텍스트, 이어쓰기
    result.open(QFile::WriteOnly|QFile::Text);// 쓰기 전용, 텍스트
   
    QTextStream SaveFile(&result);
    SaveFile << "################ RESULT ################" << endl;
    SaveFile << "F/W VERSION "      <<  "\t" << ": " << stResult.fw_version << endl;
    SaveFile << "ETH0  "            <<  "\t\t" << ": DD = "<< stResult.DD_Eth0 << ", ST = " << stResult.ST_Eth0 << endl; 
    SaveFile << "ETH1  "            <<  "\t\t" << ": DD = "<< stResult.DD_Eth1 << ", ST = " << stResult.ST_Eth1 << endl; 
    SaveFile << "USB_20  "          <<  "\t" << ": DD = "<< stResult.DD_USB_F2_0 << ", ST = " << stResult.ST_USB_F2_0 << endl; 
    SaveFile << "USB_30  "          <<  "\t" << ": DD = "<< stResult.DD_USB_F3_0 << ", ST = " << stResult.ST_USB_F3_0 << endl; 
    SaveFile << "LED POWER  "       <<  "\t" << ": " << stResult.LED_PWR << endl; 
    SaveFile << "LED SYSTEM  "      <<  "\t" << ": " << stResult.LED_SYS << endl; 
    SaveFile << "LED CAMERA  "      <<  "\t" << ": " << stResult.LED_CAM << endl; 
    SaveFile << "LED IR/SWITCH  "   <<  "\t" << ": " << stResult.LED_IR << endl << endl; 
    if(stResult.ST_Eth0 && stResult.ST_Eth1 && stResult.ST_USB_F2_0 && stResult.ST_USB_F3_0 && stResult.LED_PWR && stResult.LED_SYS && stResult.LED_CAM && stResult.LED_IR)
    {
        SaveFile << "CHECK RESULT "     <<  "\t" << ": " << "OK" << endl; 

        ui->label_13->setStyleSheet("QLabel { background-color : green; color : black; }");
        ui->label_13->setText("OK");
    }
    else
    {
        SaveFile << "CHECK RESULT "     <<  "\t" << ": " << "NG" << endl; 
        ui->label_13->setStyleSheet("QLabel { background-color : red; color : black; }");
        ui->label_13->setText("NG");
    }

    SaveFile << "########################################" << endl;

    result.close(); // 파일닫기 

    
    rename("/oem/tmp_report.txt", "/oem/report_sysmon.txt");
}

void MainWindow::updateMacAddress()
{
    QProcess pMac;
    pMac.start("sh");
    pMac.write("ifconfig -a | grep eth0 | awk \'{print \$5\}\'");
    pMac.closeWriteChannel();
    pMac.waitForFinished();
    QString output(pMac.readAllStandardOutput());
    //qDebug() << output;
    //cout << output.toStdString() << endl;

    // 이더넷 디바이스 체크
    if (output.isEmpty())   // not found
    {
        ui->label_mac->setStyleSheet("QLabel { background-color : red; color : black; }");
        ui->label_mac->setText("unknown");
    }
    else
    {
        ui->label_mac->setStyleSheet("QLabel { background-color : green; color : black; }");
        output.remove(QChar('\n'), Qt::CaseInsensitive);
        ui->label_mac->setText(output);
    }
}

void MainWindow::initPIPE()
{
#ifdef __linux__
#elif _WIN32
#else
#endif
}

void MainWindow::readPIPE()
{

}

void MainWindow::writePIPE()
{

}

void MainWindow::closePIPE()
{

}



void MainWindow::Run(int data)
{
    //qDebug() << data;

    // 프로그레스바
    updateProgressBar();

    // IPC - pipe
    readPIPE();
}

void MainWindow::updateEth0()
{
    QProcess pEth0;
    pEth0.start("ifconfig eth0");
    pEth0.waitForFinished();
    QString output(pEth0.readAllStandardOutput());
    //qDebug() << output;
    //cout << output.toStdString() << endl;

    // 이더넷 디바이스 체크
    if (output.isEmpty() || output.contains("Device not found", Qt::CaseInsensitive))   // not found
    {
        ui->label_eth0->setStyleSheet("QLabel { background-color : red; color : white; }");
        ui->label_eth0->setText("not found");

        stResult.DD_Eth0 = 0;
    }
    // 상태 정보 전송
    else
    {
        stResult.DD_Eth0 = 1;

        ui->label_eth0->setStyleSheet("QLabel { background-color : green; color : white; }");
        ui->label_eth0->setText("OK");

        // 접속상태 체크(Link up/down)
        pEth0.start("sh");
        pEth0.write("cat /sys/class/net/eth0/operstate");
        pEth0.closeWriteChannel();
        pEth0.waitForFinished();
        QString output(pEth0.readAllStandardOutput());
        //cout << output.toStdString() << endl;
        if (output.isEmpty() || output.contains("up", Qt::CaseInsensitive))     // connected
        {
            stResult.ST_Eth0 = 1;

            //ui->label_eth0_conn->show();
            ui->label_eth0_conn->setStyleSheet("QLabel { background-color : green; color : white; }");
            ui->label_eth0_conn->setText("Connected");
        }
        else                                                                    // not connected
        {
            //stResult.ST_Eth0 = 0;

            //ui->label_eth0_conn->hide();
            ui->label_eth0_conn->setStyleSheet("QLabel { background-color : gray; color : white; }");
            ui->label_eth0_conn->setText("ready...");
        }

	// rem. by ariari : 2022.10.12
        //updateMacAddress();
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
    //cout << output.toStdString() << endl;

    // 이더넷 디바이스 체크
    if (output.isEmpty() || output.contains("Device not found", Qt::CaseInsensitive))   // not found
    {
        ui->label_eth1->setStyleSheet("QLabel { background-color : red; color : white; }");
        ui->label_eth1->setText("not found");
    	    
        ui->label_eth1_conn->setStyleSheet("QLabel { background-color : gray; color : white; }");
        ui->label_eth1_conn->setText("ready...");

        stResult.DD_Eth1 = 0;
    }
    // 상태 정보 전송
    else
    {
        stResult.DD_Eth1 = 1;

        ui->label_eth1->setStyleSheet("QLabel { background-color : green; color : white; }");
        ui->label_eth1->setText("OK");

    	pEth1.start("sh");
    	pEth1.write("cat /sys/class/net/eth1/operstate");
    	pEth1.closeWriteChannel();
    	pEth1.waitForFinished();
    	QString output(pEth1.readAllStandardOutput());
    	//cout << output.toStdString() << endl;
    	if (output.isEmpty() || output.contains("up", Qt::CaseInsensitive))     // connected
    	{
            stResult.ST_Eth1 = 1;

    	    //ui->label_eth1_conn->show();
    	    ui->label_eth1_conn->setStyleSheet("QLabel { background-color : green; color : white; }");
    	    ui->label_eth1_conn->setText("Connected");
    	}
    	else                                                                    // not connected
    	{
            //stResult.ST_Eth1 = 0;

    	    //ui->label_eth1_conn->hide();
    	    ui->label_eth1_conn->setStyleSheet("QLabel { background-color : gray; color : white; }");
    	    ui->label_eth1_conn->setText("ready...");
    	}
    }
    	
    // 종료
    pEth1.close();
}

// Front 2.0 (Bus 1 and 2)
void MainWindow::updateUsb0()
{
    QProcess pUsb0;
    pUsb0.start("sh");
    pUsb0.write("lsusb | grep \"Bus 001\\\|Bus 002\"");
    //pUsb0.write("lsusb | grep \"Bus 003\\\|Bus 004\"");
    pUsb0.closeWriteChannel();
    pUsb0.waitForFinished();
    QString output(pUsb0.readAllStandardOutput());
    //qDebug() << output;
    //cout << output.toStdString() << endl;

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

        stResult.DD_USB_F2_0 = 0;
    }
    else if (tree_info_cnt == 2)
    {
        stResult.DD_USB_F2_0 = 1;

        ui->label_usb_f2->setStyleSheet("QLabel { background-color : green; color : white; }");
        ui->label_usb_f2->setText("OK");
    }


    // USB 연결정보
    if (dev_info_cnt == 1)
    {
        stResult.ST_USB_F2_0 = 1;

        //ui->label_usb_f2_conn->show();
        ui->label_usb_f2_conn->setStyleSheet("QLabel { background-color : green; color : white; }");
        ui->label_usb_f2_conn->setText("Connected");
    }
    else
    {
        //stResult.ST_USB_F2_0 = 0;

        //ui->label_usb_f2_conn->hide();
        ui->label_usb_f2_conn->setStyleSheet("QLabel { background-color : gray; color : white; }");
        ui->label_usb_f2_conn->setText("ready...");
    }

    // 종료
    pUsb0.close();
}

// Front 3.0 (Bus 7 and 8)              : rem. by ariari : 2022.07.18
// Front 3.0 (Bus 5 and 6) (eMMC)       : add. by ariari : 2022.07.18
void MainWindow::updateUsb1()
{
    QProcess pUsb1;
    pUsb1.start("sh");
    pUsb1.write("lsusb | grep \"Bus 005\\\|Bus 006\"");
    //pUsb1.write("lsusb | grep \"Bus 007\\\|Bus 008\""); // add. by ariari : 2022.07.18 => assigned to USB to Ethernet
    pUsb1.closeWriteChannel();
    pUsb1.waitForFinished();
    QString output(pUsb1.readAllStandardOutput());
    //qDebug() << output;
    //cout << output.toStdString() << endl;

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
        stResult.DD_USB_F3_0 = 0;

        ui->label_usb_f3->setStyleSheet("QLabel { background-color : red; color : white; }");
        ui->label_usb_f3->setText("not found");
    }
    else if (tree_info_cnt == 2)
    {
        stResult.DD_USB_F3_0 = 1;

        ui->label_usb_f3->setStyleSheet("QLabel { background-color : green; color : white; }");
        ui->label_usb_f3->setText("OK");
    }


    // USB 연결정보
    if (dev_info_cnt == 1)
    {
        stResult.ST_USB_F3_0 = 1;

        //ui->label_usb_f3_conn->show();
        ui->label_usb_f3_conn->setStyleSheet("QLabel { background-color : green; color : white; }");
        ui->label_usb_f3_conn->setText("Connected");
    }
    else
    {
        //stResult.ST_USB_F3_0 = 0;

        //ui->label_usb_f3_conn->hide();
        ui->label_usb_f3_conn->setStyleSheet("QLabel { background-color : gray; color : white; }");
        ui->label_usb_f3_conn->setText("ready...");
    }

    // 종료
    pUsb1.close();
}

// Rear 3.0 (Bus 5 and 6)                       : rem. by ariari : 2022.07.18
// Rear 3.0 (Bus 7 and 8) (USB to Ethernet)     : add. by ariari : 2022.07.18
void MainWindow::updateUsb2()
{
    QProcess pUsb2;
    pUsb2.start("sh");
    //pUsb2.write("lsusb | grep \"Bus 005\\\|Bus 006\"");
    pUsb2.write("lsusb | grep \"Bus 007\\\|Bus 008\""); // add. by ariari : 2022.07.18 => assigned to USB to Ethernet
    pUsb2.closeWriteChannel();
    pUsb2.waitForFinished();
    QString output(pUsb2.readAllStandardOutput());
    //qDebug() << output;
    //cout << output.toStdString() << endl;

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

    // 종료
    pUsb2.close();
}

void MainWindow::updateGPIOs()
{
    QProcess pGpio;

    // FD_LED_SYSTEM
    pGpio.start("sh");
    QString dev_sys = QString("cat %1").arg(FD_LED_SYSTEM);
    pGpio.write(dev_sys.toStdString().c_str());
    pGpio.closeWriteChannel();
    pGpio.waitForFinished();
    QString output_sys(pGpio.readAllStandardOutput());
    //pGpio.close();

    // FD_LED_CAMERA
    pGpio.start("sh");
    QString dev_cam = QString("cat %1").arg(FD_LED_CAMERA);
    pGpio.write(dev_cam.toStdString().c_str());
    pGpio.closeWriteChannel();
    pGpio.waitForFinished();
    QString output_cam(pGpio.readAllStandardOutput());
    //pGpio.close();

    // FD_LED_SWITCH
    pGpio.start("sh");
    QString dev_ir = QString("cat %1").arg(FD_LED_SWITCH);
    pGpio.write(dev_ir.toStdString().c_str());
    pGpio.closeWriteChannel();
    pGpio.waitForFinished();
    QString output_ir(pGpio.readAllStandardOutput());
    pGpio.close();

    int led_pwr = 1;
    int led_sys = output_sys.toInt();
    int led_cam = output_cam.toInt();
    int led_ir = output_ir.toInt();
#if 0
    stResult.LED_PWR    = 1;
    stResult.LED_SYS    = output_sys.toInt();
    stResult.LED_CAM    = output_cam.toInt();
    stResult.LED_IR     = output_ir.toInt();
#else
    stResult.LED_PWR    = led_pwr;
    stResult.LED_SYS    = stResult.LED_SYS == 0 ? led_sys : stResult.LED_SYS;
    stResult.LED_CAM    = stResult.LED_CAM == 0 ? led_cam : stResult.LED_CAM;
    stResult.LED_IR     = stResult.LED_IR ==0 ? led_ir : stResult.LED_IR;
#endif

    if(led_pwr == 1)
    {
        ui->label_PWR->setStyleSheet("QLabel { background-color : green; color : black; }");
    }
    else
    {
        ui->label_PWR->setStyleSheet("QLabel { background-color : gray; color : black; }");
    }
    if(led_sys == 1)
    {
        ui->label_SYS->setStyleSheet("QLabel { background-color : yellow; color : black; }");    
    }
    else
    {
        ui->label_SYS->setStyleSheet("QLabel { background-color : gray; color : black; }");    
    }
    if(led_cam == 1)
    {
        ui->label_CAM->setStyleSheet("QLabel { background-color : red; color : black; }");    
    }
    else
    {
        ui->label_CAM->setStyleSheet("QLabel { background-color : gray; color : black; }");    
    }
    if(led_ir == 1)
    {
        ui->label_SW->setStyleSheet("QLabel { background-color : green; color : black; }");
    }
    else
    {
        ui->label_SW->setStyleSheet("QLabel { background-color : gray; color : black; }");
    }


    //cout << "LEDs : " << stResult.LED_IR << ", " << stResult.LED_CAM << ", " << stResult.LED_SYS << ", " << stResult.LED_PWR << endl;
  
    //QString strHealthReport;
    //strHealthReport.sprintf("LEDs : %d:%d:%d:%d", stResult.LED_IR, stResult.LED_CAM, stResult.LED_SYS, stResult.LED_PWR);
    //INFO_LOG(strHealthReport.toStdString());

}
