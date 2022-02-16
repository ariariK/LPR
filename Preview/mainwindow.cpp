#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFileDialog>
#include <QDebug>
#include <QDesktopWidget>

#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "Spinnaker.h"
#include "SpinGenApi/SpinnakerGenApi.h"

using namespace Spinnaker;
using namespace Spinnaker::GenApi;
using namespace Spinnaker::GenICam;

#define LOG_NAME	"[Preview]"

#define IPC_SM  0
#define IPC_MQ  1
#define IPC_MODE IPC_MQ

#define MARGIN_HOR 18
#define MARGIN_VER 18

std::map<std::string, std::string> hangul_dict = {
        { "가", "10" },{ "나", "11" },{ "다", "12" },{ "라", "13" },{ "마", "14" },
        { "거", "15" },{ "너", "16" },{ "더", "17" },{ "러", "18" },{ "머", "19" },
        { "버", "20" },{ "서", "21" },{ "어", "22" },{ "저", "23" },{ "고", "24" },
        { "노", "25" },{ "도", "26" },{ "로", "27" },{ "모", "28" },{ "보", "29" },
        { "소", "30" },{ "오", "31" },{ "조", "32" },{ "구", "33" },{ "누", "34" },
        { "두", "35" },{ "루", "36" },{ "무", "37" },{ "부", "38" },{ "수", "39" },
        { "우", "40" },{ "주", "41" },{ "하", "42" },{ "허", "43" },{ "호", "44" },
        { "바", "45" },{ "사", "46" },{ "아", "47" },{ "자", "48" },{ "배", "49" },
        { "서울", "50" },{ "부산", "51" },{ "대구", "52" },{ "인천", "53" },{ "광주", "54" },
        { "대전", "55" },{ "울산", "56" },{ "세종", "57" },{ "경기", "58" },{ "강원", "59" },
        { "충북", "60" },{ "충남", "61" },{ "전북", "62" },{ "전남", "63" },{ "경북", "64" },
        { "경남", "65" },{ "제주", "66" },
        // add. @ ver20211115
        { "서울", "67" },{ "부산", "68" },{ "대구", "69" },{ "인천", "70" },{ "광주", "71" },
        { "대전", "72" },{ "울산", "73" },{ "세종", "74" },{ "경기", "75" },{ "강원", "76" },
        { "충북", "77" },{ "충남", "78" },{ "전북", "79" },{ "전남", "80" },{ "경북", "81" },
        { "경남", "82" },{ "제주", "83" },
    };

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // 동작시간
    startTime = time(NULL); // 현재 시간을 받음

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

    openlog(LOG_NAME, LOG_PID, LOG_USER);

#if true
    setStyleSheet("QWidget#MainWindow { background-color : black; color : white; }");
    setAutoFillBackground( true );
    statusBar()->hide();

    ui->screen->setStyleSheet("QLabel { background-color : black; color : red; }");
    //ui->screen->setStyleSheet("QLabel { background-color : blue; color : white; }");

    QRect rec = QApplication::desktop()->screenGeometry();
    screenWidth     = rec.width() - MARGIN_HOR;
    screenHeight    = rec.height() - MARGIN_VER;
    ui->screen->setMinimumSize(screenWidth, screenHeight);
    ui->screen->setMaximumSize(screenWidth, screenHeight);
    
    // font
    QFont font = ui->screen->font();
    font.setPointSize(20);
    font.setBold(true);
    ui->screen->setFont(font);

    //font_lpr.setPixelSize(64);
    font_lpr.setPixelSize(48);
    font_lpr.setBold(true);

    // debug font
    font_dbg.setPixelSize(20);
    font_dbg.setBold(true);
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
    Sm_Lpr = new Ipcs(KEY_NUM_SM_LPR, MEM_SIZE_SM_LPR);
    Sm_Lpr->SharedMemoryInit();

    Sm_Cam = new Ipcs(KEY_NUM_SM_CAM, MEM_SIZE_SM_CAM);
    Sm_Cam->SharedMemoryInit();

    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(Update()));
    //timer->start(200);
    timer->start(50);

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
    SAFE_DELETE(Sm_Lpr);
    SAFE_DELETE(Sm_Cam);

    closelog();

    delete ui;
}

void MainWindow::GetParameters()
{
    capWidth    = 0;
    capHeight   = 0;
}

void MainWindow::CheckScreenGeometry()
{
    // add. by ariari : 2022.02.16 - begin
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
    // add. by ariari : 2022.02.16 - end


    QRect rec = QApplication::desktop()->screenGeometry();
    if (screenWidth != rec.width() || screenHeight != rec.height())
    {
        screenWidth     = rec.width() - MARGIN_HOR;
        screenHeight    = rec.height() - MARGIN_VER;
        ui->screen->setMinimumSize(screenWidth, screenHeight);
        ui->screen->setMaximumSize(screenWidth, screenHeight);

#if true
        int offset_x = 20;
        rect_city.setRect((2*ui->screen->width())/3 + offset_x, ui->screen->height() - 190,  (ui->screen->width()/3)*0.2, 200);
        rect_left.setRect((2*ui->screen->width())/3 + (ui->screen->width()/3)*0.2 + offset_x, ui->screen->height() - 190,  (ui->screen->width()/3)*0.25, 200);
        rect_center.setRect((2*ui->screen->width())/3 + (ui->screen->width()/3)*0.45 + offset_x, ui->screen->height() - 190,  (ui->screen->width()/3)*0.1, 200);
        rect_right.setRect((2*ui->screen->width())/3 + (ui->screen->width()/3)*0.55 + offset_x, ui->screen->height() - 190,  (ui->screen->width()/3)*0.35, 200);
#else
        rect_left.setRect((2*ui->screen->width())/3, ui->screen->height() - 190,  (ui->screen->width()/3)*0.3, 200);
        rect_center.setRect((2*ui->screen->width())/3 + (ui->screen->width()/3)*0.3, ui->screen->height() - 190,  (ui->screen->width()/3)*0.2, 200);
        rect_right.setRect((2*ui->screen->width())/3 + (ui->screen->width()/3)*0.5, ui->screen->height() - 190,  (ui->screen->width()/3)*0.5, 200);
#endif        
    }
}

QString MainWindow::GetCPUTemp()
{
    QFile file("/sys/class/thermal/thermal_zone0/temp");
    if(!file.open(QFile::ReadOnly | QFile::Text))
    {
        qDebug() << " Could not open the file for reading";
        return "";
    }

    QTextStream in(&file);
    //QString myText = in.readAll();
    QString myText = in.readLine();
    //qDebug() << myText;
    file.close();

    return QString::number((myText.toDouble()/1000), 'f', 2);
}

QString MainWindow::GetRunningTime()
{
    time_t curTime;

    curTime = time(NULL);              // 현재 시간을 받음
    double diff_timer = difftime(curTime, startTime);

    int day = (int)(diff_timer/60)/60/24;
    int hour = (int)((diff_timer/60)/60)%24;
    int min = (int)(diff_timer/60)%60;
    int sec = (int)diff_timer%60;

    QString strElapsedTime;
    strElapsedTime.sprintf("%02d:%02d:%02d:%02d", day, hour, min, sec);

    return strElapsedTime;
}

void MainWindow::GetCarNoInfo(string strCarNo)
{
    int i;
    int size = 0;
    int ver = 0;        // 0 - old version (지역정보)
    int len = strlen(strCarNo.c_str());

    strHangulF = "00";    // 지역정보
    strNoFront = "";
    strHangulB = "0";
    strNoBack  = "";
    //printf("len : %d\n", len);
    if (len < 9) 
    {
        strHangulFileNameF = "/oem/hangul/" + hangul_dict[strHangulF] + ".jpg";   
        strHangulFileNameB = "/oem/hangul/" + hangul_dict[strHangulB] + ".jpg";
        return;
    }

    // 한글 2글자가 앞에 위치(구버전 번호판)    
    for(i=0; i<6; i++)
    {
        if((carNo[i] & 0x80) != 0x80)   // 숫자
        {
            ver = 1;    // new version
            break;
        }
    }
    //printf("ver : %d\n", ver);

    if (ver == 1)       // new(current) version(지역정보 없음)
    {
        // 1. 지역정보(없음)
        strHangulF = "00";
        for(i=0; i<len; i++)
        {
            // 2. 첫번째 숫자부분(2~3자리)
            if((carNo[i] & 0x80) == 0x80)   // 한글체크(한글이 시작하는 위치 찾기)
            {
                size = i;
                strNoFront = strCarNo.substr(0, size);
                break;
            }
        }
        if(size < 2) return;

        // 2. 한글(필수)
        if(i+3 > (len-4)) return;
        strHangulB = strCarNo.substr(i, 3);      // 한글1자, 3자리

        // 3. 숫자(4자리)
        if(i+7 > len) return;
        strNoBack = strCarNo.substr(i+3, 4);

    }
    else                // old version(지역정보포함)
    {
        // 1. 지역정보(있음)
        strHangulF = strCarNo.substr(0, 6);     // 한글2자, 6자리
        for(i=6; i<len; i++)
        {
            // 2. 첫번째 숫자부분(2~3자리)
            if((carNo[i] & 0x80) == 0x80)   // 한글체크(한글이 시작하는 위치 찾기)
            {
                size = i-6;
                strNoFront = strCarNo.substr(i-size, size);     // mod. by ariari : 2021.11.25 bug fix : i => i-size
                break;
            }
        }
        //printf("i = %d, size = %d, strNoFront = %s\n", i, size, strNoFront.c_str());
        if(size < 2) return;
        
        // 2. 한글(필수)
        if(i+3 > (len-4)) return;
        strHangulB = strCarNo.substr(i, 3);      // 한글1자, 3자리

        // 3. 숫자(4자리)
        if(i+7 > len) return;
        strNoBack = strCarNo.substr(i+3, 4);
    }
    strHangulFileNameF = "/oem/hangul/" + hangul_dict[strHangulF] + ".jpg";   
    strHangulFileNameB = "/oem/hangul/" + hangul_dict[strHangulB] + ".jpg";   
 }

void MainWindow::Update()
{
    //qDebug() << "Update";

#if (IPC_MODE == IPC_SM)
    Sm_Res->SharedMemoryRead((char *)&st_grab);
    capWidth    = st_grab.capWidth;
    capHeight   = st_grab.capHeight;
    msg = string_format("st_grab.capWidth = %d, st_grab.capHeight = %d, capWidth = %d, capHeight = %d\n", st_grab.capWidth, st_grab.capHeight, capWidth, capHeight);
    INFO_LOG(msg);
#elif (IPC_MODE == IPC_MQ)
    //if (!capWidth || !capHeight)
    if (capWidth <= 0|| capHeight <= 0)
    {
        for(int i=0; i<10; i++)
        {
            if( Mq_Grab->MessageQueueRead((char*)&msq) > 0 )     // non-blocking mode(IPC_NOWAIT)
            {
                capWidth    = msq.data.capWidth;
                capHeight   = msq.data.capHeight;
                break;
            }
            usleep(1000000);
            Mq_Grab->MessageQueueInit();
            continue;
        }
        msg = string_format("msq.data.capWidth = %d, msq.data.capHeight = %d, capWidth = %d, capHeight = %d\n", msq.data.capWidth, msq.data.capHeight, capWidth, capHeight);
        INFO_LOG(msg);
    }
#else
#endif

    // add. by ariari : 2022.02.16
    CheckScreenGeometry();  

    if (capWidth > 0 && capHeight > 0)
    {
        // rem. by ariari : 2022.02.16
        //CheckScreenGeometry();  

#if (IPC_MODE == IPC_SM)
#elif (IPC_MODE == IPC_MQ)
        if (Mq_Grab->MessageQueueRead((char*)&msq, 0) < 0)      // blocking mode(0)
        {
            capWidth = 0;
            capHeight = 0;
            ERR_LOG(string("[Error!!!]MessageQueueRead()"));
        }
#else
#endif

        // Read Image data
        if (Sm_Grab->SharedMemoryRead((char *)buffer) < 0)
        {
            Sm_Grab->SharedMemoryInit();
            ERR_LOG(string("[Error!!!]SharedMemoryRead()"));
        }

        //Mat cvimg = cv::Mat(convertedImage->GetHeight(), convertedImage->GetWidth(), CV_8UC1, convertedImage->GetData(), convertedImage->GetStride());
        Mat cvimg = cv::Mat(capHeight, capWidth, CV_8UC1, (char *)buffer);
        //cv::putText(cvimg, version.toLocal8Bit().data(), cv::Point(msq.data.capWidth - 380, 40), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(255, 0, 0), 4);

        // background
        //cv::rectangle(cvimg, Rect(Point(20,10) + Point(0,50), Point(20,10) + Point(400,0)), Scalar(0,0,0),-1);
        cv::rectangle(cvimg, Rect(Point(20,10), Point(400,60)), Scalar(0,0,0),-1);
        cv::putText(cvimg, version.toLocal8Bit().data(), cv::Point(40, 40), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(255, 0, 0), 4);


#if false   // single view mode
        // ���÷��� fix to window
        QPixmap picture = QPixmap::fromImage(QImage((unsigned char*) cvimg.data,
                                        cvimg.cols,
                                        cvimg.rows,
                                        QImage::Format_Grayscale8));


        ui->screen->setPixmap(picture.scaled(ui->screen->size(), Qt::KeepAspectRatio));
#else       // multi view mode
        if (Sm_Lpr->SharedMemoryRead((char *)carNo) < 0)
        {
            Sm_Lpr->SharedMemoryInit();
        }
        string strCarNo(carNo);
        //string strCarNo("12가3456");  // for test
        //cout << "carNo result : " << carNo << ", len = " << strlen(carNo) << endl;

        QPixmap picture = QPixmap::fromImage(QImage((unsigned char*) cvimg.data,
                                    cvimg.cols,
                                    cvimg.rows,
                                    QImage::Format_Grayscale8));

        QPixmap *pixmap     = new QPixmap(ui->screen->width(), ui->screen->height());
        QPainter *painter   = new QPainter(pixmap);
        pixmap->fill(Qt::transparent);
        painter->drawRect(rect_left);
        painter->setPen(QColor(Qt::yellow));
        painter->setFont(font_lpr);
       
        // top(org)
        painter->drawPixmap(0, 0, ui->screen->width(), ui->screen->height() - 200, picture);

        // left(org-small)
        //painter->drawPixmap(0, ui->screen->height() - 190, ui->screen->width()/2, 200, QPixmap("/oem/Screen_shot/0.jpg"));
        painter->drawPixmap(0, ui->screen->height() - 190, ui->screen->width()/3, 200, QPixmap("/oem/Screen_shot/0.jpg"));

        // center(lpd)
        //painter->drawPixmap(ui->screen->width()/2, ui->screen->height() - 190, ui->screen->width()/2, 200, QPixmap("/oem/Screen_shot/1.jpg"));
        painter->drawPixmap(ui->screen->width()/3, ui->screen->height() - 190, ui->screen->width()/3, 200, QPixmap("/oem/Screen_shot/1.jpg"));

        // right(string)
        GetCarNoInfo(strCarNo);
        painter->drawPixmap(rect_city.x(), rect_center.y()+76, rect_city.width(), 48, QPixmap(strHangulFileNameF.c_str()));
        painter->drawText(rect_left, Qt::AlignHCenter | Qt::AlignVCenter, strNoFront.c_str());
        painter->drawPixmap(rect_center.x(), rect_center.y()+76, rect_center.width(), 48, QPixmap(strHangulFileNameB.c_str()));
        painter->drawText(rect_right, Qt::AlignHCenter | Qt::AlignVCenter, strNoBack.c_str());

        // debug message(right/top)
        if (QFile::exists("/media/usb0/debug_enable"))
        {
            if (Sm_Cam->SharedMemoryRead((char *)&st_cam) < 0)
            {
                Sm_Cam->SharedMemoryInit();
            }

            QRectF debug_rect(screenWidth/2, 20, screenWidth/2-20, screenHeight-240);
            //painter->setPen(QColor(Qt::yellow));
            painter->setFont(font_dbg);
            painter->setBrush(QColor(0, 0, 0));
            painter->drawRect(debug_rect);

            msg = string_format(" Program Running Time : %s", GetRunningTime().toStdString().c_str());
            painter->drawText(debug_rect, Qt::AlignTop | Qt::AlignLeft, msg.c_str());
            
            debug_rect.setY(debug_rect.y()+30);
            msg = string_format(" Total Captured Frame : %ld, %.2f[FPS]", st_cam.capCount, st_cam.capFPS);
            painter->drawText(debug_rect, Qt::AlignTop | Qt::AlignLeft, msg.c_str());

            debug_rect.setY(debug_rect.y()+30);
            msg = string_format(" Target Clock Value : %ld[Hz]", st_cam.tarClk);
            painter->drawText(debug_rect, Qt::AlignTop | Qt::AlignLeft, msg.c_str());

            debug_rect.setY(debug_rect.y()+30);
            msg = string_format(" Exposure - MIN : %.2f", st_cam.expMin);
            painter->drawText(debug_rect, Qt::AlignTop | Qt::AlignLeft, msg.c_str());
            debug_rect.setY(debug_rect.y()+25);
            msg = string_format(" Exposure - MAX : %.2f", st_cam.expMax);
            painter->drawText(debug_rect, Qt::AlignTop | Qt::AlignLeft, msg.c_str());
            
            debug_rect.setY(debug_rect.y()+30);
            msg = string_format(" Shutter - MIN : %.2f[msec]", st_cam.shMin/1000);
            painter->drawText(debug_rect, Qt::AlignTop | Qt::AlignLeft, msg.c_str());
            debug_rect.setY(debug_rect.y()+25);
            msg = string_format(" Shutter - MAX : %.2f[msec]", st_cam.shMax/1000);
            painter->drawText(debug_rect, Qt::AlignTop | Qt::AlignLeft, msg.c_str());
            
            debug_rect.setY(debug_rect.y()+30);
            msg = string_format(" Gain - MIN : %.2f", st_cam.gainMin);
            painter->drawText(debug_rect, Qt::AlignTop | Qt::AlignLeft, msg.c_str());
            debug_rect.setY(debug_rect.y()+25);
            msg = string_format(" Gain - MAX : %.2f[dB]", st_cam.gainMax);
            painter->drawText(debug_rect, Qt::AlignTop | Qt::AlignLeft, msg.c_str());
            
            debug_rect.setY(debug_rect.y()+30);
            msg = string_format(" Current DN(IR) Status : %s", st_cam.dnStatus ? "OFF" : "ON");
            painter->drawText(debug_rect, Qt::AlignTop | Qt::AlignLeft, msg.c_str());
            debug_rect.setY(debug_rect.y()+25);
            msg = string_format(" Current Exposure Value : %.2f", st_cam.expCur);
            painter->drawText(debug_rect, Qt::AlignTop | Qt::AlignLeft, msg.c_str());
            debug_rect.setY(debug_rect.y()+25);
            msg = string_format(" Current Shutter Value : %.3f[msec]", st_cam.shCur/1000);
            painter->drawText(debug_rect, Qt::AlignTop | Qt::AlignLeft, msg.c_str());
            debug_rect.setY(debug_rect.y()+25);
            msg = string_format(" Current Gain Value : %.3f[dB]", st_cam.gainCur);
            painter->drawText(debug_rect, Qt::AlignTop | Qt::AlignLeft, msg.c_str());

            debug_rect.setY(debug_rect.y()+30);
            msg = string_format(" CPU Temperature : %s", GetCPUTemp().toStdString().c_str());
            painter->drawText(debug_rect, Qt::AlignTop | Qt::AlignLeft, msg.c_str());
            

        }

#if false
        carNo[0] = 51;
        carNo[1] = 51;
        carNo[2] = 234;
        carNo[3] = 176;
        carNo[4] = 128;
        carNo[5] = 57;
        carNo[6] = 57;
        carNo[7] = 57;
        carNo[8] = 57;
        carNo[9] = 0;
        printf("[len] : %d\n", strlen(carNo));
        int h_start = 0;
        int len = strlen(carNo);
        char carNo_front[8];
        char carNo_back[8];
        int i;
        for(i=0; i<len; i++)
        {
            carNo_front[i] = carNo[i];
            if((carNo[i] & 0x80) == 0x80)
            {
                carNo_front[i] = 0;
                h_start = i;
                break;
            }
        }
        printf("[carNo_front : %d] : %s\n", h_start, carNo_front);
        carNo_back[0] = carNo[len-4];
        carNo_back[1] = carNo[len-3];
        carNo_back[2] = carNo[len-2];
        carNo_back[3] = carNo[len-1];
        carNo_back[4] = 0;

        strHangul = strCarNo.substr(h_start, max(0, len - 4 - h_start));
        strHangulFileName = "/oem/" + hangul_dict[strHangul] + ".jpg";

        
        //painter->drawText(rect, Qt::AlignHCenter | Qt::AlignVCenter, carNo);
        painter->drawText(rect_left, Qt::AlignHCenter | Qt::AlignVCenter, carNo_front);

        //painter->drawRect(rect2);
        //painter->drawPixmap(rect2.x(), rect2.y()+76, rect2.width(), 48, QPixmap("/oem/A.jpg"));
        painter->drawPixmap(rect_center.x(), rect_center.y()+76, rect_center.width(), 48, QPixmap(strHangulFileName.c_str()));
        //painter->drawPixmap(rect2.x(), rect2.y()+76, rect2.width(), 48, QPixmap(strFile.c_str()));
        //painter->drawPixmap(rect2.x(), rect2.y()+76, rect2.width(), 48, QPixmap("/oem/'$'\352\260\200''.jpg"));

        //painter->drawRect(rect3);
        //painter->setPen(QColor(Qt::yellow));
        //painter->setFont(font_lpr);
        painter->drawText(rect_right, Qt::AlignHCenter | Qt::AlignVCenter, carNo_back);
#endif
        

        painter->end();

        ui->screen->setPixmap(*pixmap);
        ui->screen->resize(QSize(ui->screen->width(),ui->screen->height()));

        ui->screen->pixmap()->save("/oem/Screen_shot/monitor_tmp.jpg");
        rename("/oem/Screen_shot/monitor_tmp.jpg", "/oem/Screen_shot/monitor.jpg");

        delete pixmap;
        delete painter;
#endif
    }
    else 
    {
        // Defalut Image Display
        QString qsfileName="/oem/Screen_shot/no_signal.jpg";
        cv::Mat img = cv::imread(qsfileName.toLocal8Bit().data());
        cv::cvtColor(img, img, cv::COLOR_BGR2RGB); // invert BGR to RGB
        cv::putText(img, version.toLocal8Bit().data(), cv::Point(40, 40), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(255, 255, 255), 4);

        QPixmap picture = QPixmap::fromImage(QImage((unsigned char*) img.data,
                                            img.cols,
                                            img.rows,
                                            QImage::Format_RGB888));
        ui->screen->setPixmap(picture.scaled(ui->screen->size(), Qt::KeepAspectRatio));

        ui->screen->pixmap()->save("/oem/Screen_shot/monitor_tmp.jpg");
        rename("/oem/Screen_shot/monitor_tmp.jpg", "/oem/Screen_shot/monitor.jpg");
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

