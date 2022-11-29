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

#ifdef EN_LIST_DISP
// add. by ariari : 2022.05.20
#define DISP_OBJ_LIST
#endif

#define FD_CUSTOM_CODE	        "/etc/config/ccode" // custom code

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

    // add. by ariari : 2022.11.22 - begin
    // read custom code
    QString c_code("0");
    QString cc_file(FD_CUSTOM_CODE);
    if (QFile::exists(cc_file) == true) 
    {
        QFile file(cc_file);
        if(!file.open(QFile::ReadOnly | QFile::Text))
        {
            qDebug() << " Could not open file(custom code) for writing";
        }
        else 
        {
            QTextStream out(&file);
            c_code = out.readAll();
            c_code.chop(1);
            qDebug() << c_code;	
            file.flush();
            file.close();
        } 
    }
    // add. by ariari : 2022.11.22 - end

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
            version = version + "(" + c_code + ")"; // add. by ariari : 2022.11.22
            qDebug() << version;	
            file.flush();
            file.close();
        }
    }
    else
    {
    }

    // add. by ariari : begin - 2022.09.19
    // file name : info.txt
    // full path : /oem/info.txt
    //QString info_file("/oem/info.txt");
    //QFile f_info(info_file);
    //if (QFile::exists(info_file) == true) 
    //{   
    //    // create empty file
    //    f_info.open(QFile::WriteOnly|QFile::Text);
    //    f_info.flush();
    //    f_info.close();
    //}
    // add. by ariari : end - 2022.09.19

    openlog(LOG_NAME, LOG_PID, LOG_USER);

#if true
    setStyleSheet("QWidget#MainWindow { background-color : black; color : white; }");
    setAutoFillBackground( true );
    statusBar()->hide();

    ui->screen->setStyleSheet("QLabel { background-color : black; color : red; }");
    //ui->screen->setStyleSheet("QLabel { background-color : blue; color : white; }");

    QRect rec = QApplication::desktop()->screenGeometry();
#if false // org    // rem. by ariari : 2022.11.11
    screenWidth     = rec.width() - MARGIN_HOR;
    screenHeight    = rec.height() - MARGIN_VER;
    ui->screen->setMinimumSize(screenWidth, screenHeight);
    ui->screen->setMaximumSize(screenWidth, screenHeight);
#else               // add. by ariar : 2022.11.11
    screenWidth     = rec.width();
    screenHeight    = rec.height();
    ui->screen->setMinimumSize(screenWidth - MARGIN_HOR, screenHeight - MARGIN_VER);
    ui->screen->setMaximumSize(screenWidth - MARGIN_HOR, screenHeight - MARGIN_VER);

    int offset_x = 20;
    rect_city.setRect((2*ui->screen->width())/3 + offset_x, ui->screen->height() - 190,  (ui->screen->width()/3)*0.2, 200);
    rect_left.setRect((2*ui->screen->width())/3 + (ui->screen->width()/3)*0.2 + offset_x, ui->screen->height() - 190,  (ui->screen->width()/3)*0.25, 200);
    rect_center.setRect((2*ui->screen->width())/3 + (ui->screen->width()/3)*0.45 + offset_x, ui->screen->height() - 190,  (ui->screen->width()/3)*0.1, 200);
    rect_right.setRect((2*ui->screen->width())/3 + (ui->screen->width()/3)*0.55 + offset_x, ui->screen->height() - 190,  (ui->screen->width()/3)*0.35, 200);
#endif
    
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

#ifdef EN_LIST_DISP
    // lpd box
    lpd_box.setPixelSize(32);
    //lpd_box.setBold(true);
#endif

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

#ifdef EN_LIST_DISP
    // add. by ariari : 2022.05.20
    Mq_Lpdr_Info = new Ipcs(KEY_NUM_MQ_LPDR_INFO, 0);
    Mq_Lpdr_Info->MessageQueueInit();

    msq_lpdr_result.data.detect_num = 0;
    memset((char*)&info_bk[0], 0, sizeof(struct lpdr_info));
    memset((char*)&info_bk[1], 0, sizeof(struct lpdr_info));
    memset((char*)&info_bk[2], 0, sizeof(struct lpdr_info));
    memset((char*)&info_bk[3], 0, sizeof(struct lpdr_info));
    memset((char*)&info_bk[4], 0, sizeof(struct lpdr_info));
    memset((char*)&info_bk[5], 0, sizeof(struct lpdr_info));
    memset((char*)&info_bk[6], 0, sizeof(struct lpdr_info));
    memset((char*)&info_bk[7], 0, sizeof(struct lpdr_info));
    memset((char*)&info_bk[8], 0, sizeof(struct lpdr_info));
    memset((char*)&info_bk[9], 0, sizeof(struct lpdr_info));
    
    //Sm_Lpdr = new Ipcs(KEY_NUM_SM_LPDR, MEM_SIZE_SM_LPDR);
    //Sm_Lpdr->SharedMemoryInit();
#endif

    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(Update()));
    //timer->start(200);
    timer->start(50);   // release

#if false
    threadPreview = new ThreadPreview(this);
    threadPreview->start();
    connect(threadPreview, SIGNAL(Send()), this, SLOT(Run()));
#endif

    // add. by ariari : 2022.05.16 - begin
    // default values
    draw_en = 0;
    roi_sx = 0;
    roi_sy = 0;
    roi_w = 1920;
    roi_h = 1080;
#if false // add. by ariari : 2022.11.22 - begin    
    QFile file("/oem/config_lpr.txt");
    if(!file.open(QFile::ReadOnly | QFile::Text))
    {
        qDebug() << " Could not open config file for camera parameter";
        //return "";
    }
    else 
    {
        qDebug() << " Read config file!!!";

        QTextStream in(&file);
        while (!in.atEnd())
        {
            //QString statusTxt = in.readLine();
            string line = in.readLine().toStdString();

			line.erase(std::remove_if(line.begin(), line.end(), ::isspace), line.end());

			if (line[0] == '#' || line.empty())
			{
				continue;
			}

            int delimiterPos = line.find("=");
			auto name = line.substr(0, delimiterPos);
			auto value = line.substr(delimiterPos + 1);
			
			cout << name << " " << value << endl;
            
            if (name.compare("draw_en") == 0)
			{
				draw_en = stoi(value);
			}
            else if (name.compare("roi_sx") == 0)
			{
				roi_sx = stoi(value);
			}
            else if (name.compare("roi_sy") == 0)
            {
                roi_sy = stoi(value);
            }
            else if (name.compare("roi_w") == 0)
            {
                roi_w = stoi(value);
            }
            else if (name.compare("roi_h") == 0)
            {
                roi_h = stoi(value);
            }
        }
        file.close();

        qDebug() << " Close config file!!!";
    }
#endif // add. by ariari : 2022.11.22 - end    
    // add. by ariari : 2022.05.16 - end

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
#ifdef EN_LIST_DISP    
    // add. by ariari : 2022.05.20
    SAFE_DELETE(Mq_Lpdr_Info);
    //SAFE_DELETE(Sm_Lpdr);
#endif

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
    static int pre_hdmi_status = -1;    // 0:disconnected, 1:connected
    int hdmi_status;
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
    //if(statusTxt.compare("disconnected") == 0) return;    // HDMI연결하기 전에는 실시간영상의 하단우측 번호정보가 표시 안됨 : rem. by ariari : 2022.05.04
    // add. by ariari : 2022.02.16 - end

    // add. by ariari : 2022.11.11 - begin
    QRect rec = QApplication::desktop()->screenGeometry();
#if true    // add. by ariari : 2022.11.11
    if(statusTxt.compare("connected") == 0)
    {
        hdmi_status = 1;

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

        //if (screenWidth != rec.width() || screenHeight != rec.height())
        if (pre_hdmi_status != 1 || screenWidth != rec.width() || screenHeight != rec.height())
        {
            //printf("[000000000000000000]screenWidth : %d, screenHeight : %d, rec.width() : %d, rec.height() : %d\n", screenWidth, screenHeight, rec.width(), rec.height());

            ui->screen->setMinimumSize(screenWidth - MARGIN_HOR, screenHeight - MARGIN_VER);
            ui->screen->setMaximumSize(screenWidth - MARGIN_HOR, screenHeight - MARGIN_VER);

            int offset_x = 20;
            rect_city.setRect((2*ui->screen->width())/3 + offset_x, ui->screen->height() - 190,  (ui->screen->width()/3)*0.2, 200);
            rect_left.setRect((2*ui->screen->width())/3 + (ui->screen->width()/3)*0.2 + offset_x, ui->screen->height() - 190,  (ui->screen->width()/3)*0.25, 200);
            rect_center.setRect((2*ui->screen->width())/3 + (ui->screen->width()/3)*0.45 + offset_x, ui->screen->height() - 190,  (ui->screen->width()/3)*0.1, 200);
            rect_right.setRect((2*ui->screen->width())/3 + (ui->screen->width()/3)*0.55 + offset_x, ui->screen->height() - 190,  (ui->screen->width()/3)*0.35, 200);
        }
    }
    else
    {
        hdmi_status = 0;

        if (screenWidth != rec.width() || screenHeight != rec.height())
        {
            //printf("[1111111111111111111]screenWidth : %d, screenHeight : %d, rec.width() : %d, rec.height() : %d\n", screenWidth, screenHeight, rec.width(), rec.height());

            screenWidth     = rec.width();
            screenHeight    = rec.height();
            ui->screen->setMinimumSize(screenWidth - MARGIN_HOR, screenHeight - MARGIN_VER);
            ui->screen->setMaximumSize(screenWidth - MARGIN_HOR, screenHeight - MARGIN_VER);

            int offset_x = 20;  
            rect_city.setRect((2*ui->screen->width())/3 + offset_x, ui->screen->height() - 190,  (ui->screen->width()/3)*0.2, 200); 
            rect_left.setRect((2*ui->screen->width())/3 + (ui->screen->width()/3)*0.2 + offset_x, ui->screen->height() - 190,  (ui->screen->width()/3)*0.25, 200);
            rect_center.setRect((2*ui->screen->width())/3 + (ui->screen->width()/3)*0.45 + offset_x, ui->screen->height() - 190,  (ui->screen->width()/3)*0.1, 200);
            rect_right.setRect((2*ui->screen->width())/3 + (ui->screen->width()/3)*0.55 + offset_x, ui->screen->height() - 190,  (ui->screen->width()/3)*0.35, 200);
        }
    }
    pre_hdmi_status = hdmi_status;

#else       // rem. by ariari : 2022.11.11
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
#endif
    // add. by ariari : 2022.11.11 - end

    //QRect rec = QApplication::desktop()->screenGeometry();
    if (screenWidth != rec.width() || screenHeight != rec.height())
    {
        printf("screenWidth : %d, screenHeight : %d, rec.width() : %d, rec.height() : %d\n", screenWidth, screenHeight, rec.width(), rec.height());

        // rem. by ariari : 2022.11.11 - begin
        //screenWidth     = rec.width() - MARGIN_HOR;
        //screenHeight    = rec.height() - MARGIN_VER;
        //ui->screen->setMinimumSize(screenWidth, screenHeight);
        //ui->screen->setMaximumSize(screenWidth, screenHeight);
        // rem. by ariari : 2022.11.11 - end

        // add. by ariari : 2022.11.11 - begin
        ui->screen->setMinimumSize(screenWidth - MARGIN_HOR, screenHeight - MARGIN_VER);
        ui->screen->setMaximumSize(screenWidth - MARGIN_HOR, screenHeight - MARGIN_VER);
        // add. by ariari : 2022.11.11 - end

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
        // rem. by ariari : 2022.05.23
        //strHangulFileNameF = "/oem/hangul/" + hangul_dict[strHangulF] + ".jpg";   
        //strHangulFileNameB = "/oem/hangul/" + hangul_dict[strHangulB] + ".jpg";

        // add. by ariari : 2022.05.23
        if (hangul_dict.count(strHangulF)) {
            strHangulFileNameF = "/oem/hangul/" + hangul_dict[strHangulF] + ".jpg";   
        }
        else
        {
            strHangulFileNameF = "/oem/hangul/00.jpg";   
        }
        if (hangul_dict.count(strHangulB)) {
            strHangulFileNameB = "/oem/hangul/" + hangul_dict[strHangulB] + ".jpg";
        }
        else
        {
            strHangulFileNameB = "/oem/hangul/00.jpg";
        }

        return;
    }

    // 한글 2글자가 앞에 위치(구버전 번호판)    
    for(i=0; i<6; i++)
    {
        //if((carNo[i] & 0x80) != 0x80)   // 숫자
        if((strCarNo[i] & 0x80) != 0x80)   // 숫자  // mod. by ariari : 2022.11.21
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
            //if((carNo[i] & 0x80) == 0x80)   // 한글체크(한글이 시작하는 위치 찾기)
            if((strCarNo[i] & 0x80) == 0x80)   // 한글체크(한글이 시작하는 위치 찾기)   // mod. by ariari : 2022.11.21
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
            //if((carNo[i] & 0x80) == 0x80)   // 한글체크(한글이 시작하는 위치 찾기)
            if((strCarNo[i] & 0x80) == 0x80)   // 한글체크(한글이 시작하는 위치 찾기)   // mod. by ariari : 2022.11.21
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

    // reb. by ariari : 2022.05.23
    //strHangulFileNameF = "/oem/hangul/" + hangul_dict[strHangulF] + ".jpg";   
    //strHangulFileNameB = "/oem/hangul/" + hangul_dict[strHangulB] + ".jpg";   
    // add. by ariari : 2022.05.23
    if (hangul_dict.count(strHangulF)) {
        strHangulFileNameF = "/oem/hangul/" + hangul_dict[strHangulF] + ".jpg";   
    }
    else
    {
        strHangulFileNameF = "/oem/hangul/00.jpg";   
    }
    if (hangul_dict.count(strHangulB)) {
        strHangulFileNameB = "/oem/hangul/" + hangul_dict[strHangulB] + ".jpg";
    }
    else
    {
        strHangulFileNameB = "/oem/hangul/00.jpg";
    }
 }

 void MainWindow::WriteDebugInfo(QPainter *pPainter)
 {
    if (Sm_Cam->SharedMemoryRead((char *)&st_cam) < 0)
    {
        Sm_Cam->SharedMemoryInit();
    }

    // add. by ariari : 2022.11.22 - begin
    draw_en = st_cam.draw_en;
    roi_sx  = st_cam.roi_sx;
    roi_sy  = st_cam.roi_sy;
    roi_w   = st_cam.roi_w;
    roi_h   = st_cam.roi_h;
    // add. by ariari : 2022.11.22 - begin

    // File Save
    // file name : info.txt
    // full path : /oem/info.txt
    QString info_file("/oem/info.txt");
    QFile f_info(info_file);
    f_info.open(QFile::WriteOnly|QFile::Text);

    // 
    QTextStream SaveFile(&f_info);
    SaveFile << "RUNNING_TIME"            <<  "\t\t" << " : " << GetRunningTime().toStdString().c_str() << endl;
    SaveFile << "TOTAL_CAPTURE_COUNT"     <<  "\t" << " : "<< st_cam.capCount << endl; 
    SaveFile << "AVG_FPS"                 <<  "\t\t\t" << " : "<< st_cam.capFPS << endl; 
    SaveFile << "CPU_TEMP"                <<  "\t\t" << " : "<< GetCPUTemp().toStdString().c_str() << endl; 
    f_info.flush();
    f_info.close();


    // move from update function...
    if (pPainter != nullptr && QFile::exists("/media/usb0/debug_enable"))
    {
        QRectF debug_rect(screenWidth/2, 20, screenWidth/2-20, screenHeight-240);
        //painter->setPen(QColor(Qt::yellow));
        pPainter->setFont(font_dbg);
        pPainter->setBrush(QColor(0, 0, 0));
        pPainter->drawRect(debug_rect);

        msg = string_format(" Program Running Time : %s", GetRunningTime().toStdString().c_str());
        pPainter->drawText(debug_rect, Qt::AlignTop | Qt::AlignLeft, msg.c_str());
                
        debug_rect.setY(debug_rect.y()+30);
        msg = string_format(" Total Captured Frame : %ld, %.2f[FPS]", st_cam.capCount, st_cam.capFPS);
        pPainter->drawText(debug_rect, Qt::AlignTop | Qt::AlignLeft, msg.c_str());
        
        // rem. by ariari : 2022.11.09 : begin
        //debug_rect.setY(debug_rect.y()+30);
        //msg = string_format(" Target Clock Value : %ld[Hz]", st_cam.tarClk);
        //pPainter->drawText(debug_rect, Qt::AlignTop | Qt::AlignLeft, msg.c_str());
        // rem. by ariari : 2022.11.09 : end

        // add. by ariari : 2022.11.09 : begin
        debug_rect.setY(debug_rect.y()+30);
        msg = string_format(" Target AE Value : %d", st_cam.targetAE);
        pPainter->drawText(debug_rect, Qt::AlignTop | Qt::AlignLeft, msg.c_str());
        // add. by ariari : 2022.11.09 : end

        debug_rect.setY(debug_rect.y()+30);
        msg = string_format(" Exposure - MIN : %.2f", st_cam.expMin);
        pPainter->drawText(debug_rect, Qt::AlignTop | Qt::AlignLeft, msg.c_str());
        debug_rect.setY(debug_rect.y()+25);
        msg = string_format(" Exposure - MAX : %.2f", st_cam.expMax);
        pPainter->drawText(debug_rect, Qt::AlignTop | Qt::AlignLeft, msg.c_str());
        
        debug_rect.setY(debug_rect.y()+30);
        msg = string_format(" Shutter - MIN : %.2f[msec]", st_cam.shMin/1000);
        pPainter->drawText(debug_rect, Qt::AlignTop | Qt::AlignLeft, msg.c_str());
        debug_rect.setY(debug_rect.y()+25);
        msg = string_format(" Shutter - MAX : %.2f[msec]", st_cam.shMax/1000);
        pPainter->drawText(debug_rect, Qt::AlignTop | Qt::AlignLeft, msg.c_str());
        
        debug_rect.setY(debug_rect.y()+30);
        msg = string_format(" Gain - MIN : %.2f[dB]", st_cam.gainMin);
        pPainter->drawText(debug_rect, Qt::AlignTop | Qt::AlignLeft, msg.c_str());
        debug_rect.setY(debug_rect.y()+25);
        msg = string_format(" Gain - MAX : %.2f[dB]", st_cam.gainMax);
        pPainter->drawText(debug_rect, Qt::AlignTop | Qt::AlignLeft, msg.c_str());
        
        debug_rect.setY(debug_rect.y()+30);
        msg = string_format(" Current DN(IR) Status : %s", st_cam.dnStatus ? "OFF" : "ON");
        pPainter->drawText(debug_rect, Qt::AlignTop | Qt::AlignLeft, msg.c_str());
        debug_rect.setY(debug_rect.y()+25);
        msg = string_format(" Current Exposure Value : %.2f (%02f%%)", st_cam.expCur, st_cam.expCurPercent);
        pPainter->drawText(debug_rect, Qt::AlignTop | Qt::AlignLeft, msg.c_str());
        debug_rect.setY(debug_rect.y()+25);
        msg = string_format(" Current Shutter Value : %.3f[msec]", st_cam.shCur/1000);
        pPainter->drawText(debug_rect, Qt::AlignTop | Qt::AlignLeft, msg.c_str());
        debug_rect.setY(debug_rect.y()+25);
        msg = string_format(" Current Gain Value : %.3f[dB]", st_cam.gainCur);
        pPainter->drawText(debug_rect, Qt::AlignTop | Qt::AlignLeft, msg.c_str());

        debug_rect.setY(debug_rect.y()+30);
        msg = string_format(" CPU Temperature : %s", GetCPUTemp().toStdString().c_str());
        pPainter->drawText(debug_rect, Qt::AlignTop | Qt::AlignLeft, msg.c_str());
    }
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
        //cv::rectangle(cvimg, Rect(Point(20,10), Point(400,60)), Scalar(0,0,0),-1);
        cv::rectangle(cvimg, Rect(Point(20,10), Point(500,60)), Scalar(0,0,0),-1);  // mod. by ariari : 2022.11.22 for custom code value
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

#ifdef DISP_OBJ_LIST
        painter->drawPixmap(0, 0, (2*ui->screen->width())/3, ui->screen->height() - 200, picture);
#else
        painter->drawPixmap(0, 0, ui->screen->width(), ui->screen->height() - 200, picture);
#endif

        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        // add. by ariari : 2022.05.16 - begin
        // guide line
#ifdef DISP_OBJ_LIST
        int live_wnd_width  = (2*ui->screen->width())/3;
        int live_wnd_height = ui->screen->height() - 200;
#else        
        int live_wnd_width  = ui->screen->width();
        int live_wnd_height = ui->screen->height() - 200;
#endif        
        //int cx = capWidth;   // capture width
        //int cy = capHeight;  // capture height

        //int sx = 0;     // start x
        //int sy = 0;     // start y
        //int ex = 1920;  // end x
        //int ey = 1080;  // end y
        
        int red_offset = roi_h/3;   // 
        int adj_roi_w, adj_roi_h;   // add. by ariari : 2022.11.16
        if(draw_en || QFile::exists("/media/usb0/draw_enable"))
        {
            //printf("roi_sx:%d, roi_sy:%d, roi_w:%d, roi_h:%d\n", roi_sx, roi_sy, roi_w, roi_h);
            //roi_w = ((roi_sx+roi_w) > capWidth) ? (capWidth-roi_sx) : roi_w;
            //roi_h = ((roi_sy+roi_h) > capHeight) ? (capHeight-roi_sy) : roi_h;
            adj_roi_w = ((roi_sx+roi_w) > capWidth) ? (capWidth-roi_sx) : roi_w;
            adj_roi_h = ((roi_sy+roi_h) > capHeight) ? (capHeight-roi_sy) : roi_h;
            int rx = (live_wnd_width*roi_sx)/capWidth;      // remap x
            int ry = (live_wnd_height*roi_sy)/capHeight;    // remap y
            int rw = (live_wnd_width*adj_roi_w)/capWidth;       // remap w
            int rh = (live_wnd_height*adj_roi_h)/capHeight;     // remap h
            //painter->drawRect(0, 0, ui->screen->width() - 2, ui->screen->height() - 200);  // rectangle
            painter->drawRect(rx, ry, rw-2, rh-2);  // rectangle

            //printf("cw:%d, ch:%d, lw:%d, lh:%d, roi_w:%d, roi_h:%d, rx:%d, ry:%d, rw:%d, rh:%d\n", capWidth, capHeight, live_wnd_width, live_wnd_height, roi_w, roi_h, rx, ry, rw, rh);


            // add. by ariari : 2022.05.20
            // red line(zone)
            //int red_offset = (rh/3);
            red_offset = (rh/3);

            QPen redZone_pen(Qt::red);
            redZone_pen.setWidth(4);
            painter->setPen(redZone_pen);
            painter->drawLine(rx, ry + red_offset, rx+rw, ry + red_offset);  // 

            // return
            painter->setPen(QColor(Qt::yellow));
        }

        // add. by ariari : 2022.05.16 - end
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        // left(org-small)
        //painter->drawPixmap(0, ui->screen->height() - 190, ui->screen->width()/2, 200, QPixmap("/oem/Screen_shot/0.jpg"));
        int w1_sx = 0;
        int w1_sy = ui->screen->height() - 190;
        int w1_w  = ui->screen->width()/3;
        int w1_h  = 200;
        //painter->drawPixmap(0, ui->screen->height() - 190, ui->screen->width()/3, 200, QPixmap("/oem/Screen_shot/0.jpg"));
        painter->drawPixmap(w1_sx, w1_sy, w1_w, w1_h, QPixmap("/oem/Screen_shot/0.jpg"));

#ifdef EN_LIST_DISP        
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        // add. by ariari : 2022.05.20 - begin
    #if true   
        //float ratio_w = (float)w1_w/roi_w;
        //float ratio_h = (float)w1_h/roi_h;     
        float ratio_w = (float)w1_w/adj_roi_w;  // add. by ariari : 2022.11.16
        float ratio_h = (float)w1_h/adj_roi_h;  // add. by ariari : 2022.11.16

        QPen box_pen(Qt::red);
        box_pen.setWidth(4);
        painter->setPen(box_pen);
        if( Mq_Lpdr_Info != nullptr)
        {
            // add. by ariari : 2022.05.23
            int cnt_list = Mq_Lpdr_Info->MessageQueueQNum();
            //cout << "cnt list = " << cnt_list << endl;

            // add. by ariari : 2022.11.17 - begin
            QRectF list_rect_debug((2*ui->screen->width())/3 + (ui->screen->width()/3)*0.2 + 20, 20, (ui->screen->width()/3)*0.8, 48);
            msg = string_format(" DN(IR):%s, Exp:%.02f%%", st_cam.dnStatus ? "OFF" : "ON", st_cam.expCurPercent);
            painter->drawText(list_rect_debug, Qt::AlignTop | Qt::AlignLeft, msg.c_str());
            // add. by ariari : 2022.11.17 - end

            if(cnt_list > 0)
            {
                int pos = 0;

        #if true    // for
                do {    // add. by ariari : 2022.11.14
                    pos = 0;
                    memset((char*)&msq_lpdr_result, 0, sizeof(struct lpdr_result));
                    if(Mq_Lpdr_Info->MessageQueueRead((char*)&msq_lpdr_result) > 0)
                    {
                        int len = msq_lpdr_result.data.detect_num;
                        //cout << "[preview]msq_lpdr_result.data.detect_num = " << msq_lpdr_result.data.detect_num << endl;
                        for(int i=0; i<len; i++)
                        {
                            //cout << "i = " << pos << ", len = " << len << ", msq_lpdr_result.data.data[pos].carNo = " << msq_lpdr_result.data.data[pos].carNo << endl;
                            //msg = string_format("[PREVIEW]i = %d, len = %d, carNo = %s", i, len, msq_lpdr_result.data.data[pos].carNo);
                            //INFO_LOG(msg);
                            // claer
                            //memset((char*)&msq_lpdr_info, 0, sizeof(struct lpdr_info));
                            //memset((char*)&msq_lpdr_result, 0, sizeof(struct lpdr_result));
                            //if( Mq_Lpdr_Info->MessageQueueRead((char*)&lpdr_result) > 0 )     // non-blocking mode(IPC_NOWAIT)
                            //if(1)
                            {
                                // 최대 10(MQ_LPDR_INFO_MAX_QSIZE)
                                int box_sx = msq_lpdr_result.data.data[pos].x * ratio_w;
                                int box_sy = msq_lpdr_result.data.data[pos].y * ratio_h;
                                int box_w  = (msq_lpdr_result.data.data[pos].endX - msq_lpdr_result.data.data[pos].x) * ratio_w;
                                int box_h  = (msq_lpdr_result.data.data[pos].endY - msq_lpdr_result.data.data[pos].y) * ratio_h;


                                // draw box...(하단우측)
                                painter->setPen(box_pen);
                                painter->drawRect(w1_sx + box_sx, w1_sy + box_sy, box_w, box_h);

                #ifdef DISP_OBJ_LIST
                                strncpy(carNo, msq_lpdr_result.data.data[pos].carNo, strlen(msq_lpdr_result.data.data[pos].carNo));
                                string strCarNo_new(msq_lpdr_result.data.data[pos].carNo);
                                GetCarNoInfo(strCarNo_new);

                                ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                                // backup
                                // claer
                                memset((char*)&info_bk[pos], 0, sizeof(struct lpdr_info));
                                info_bk[pos].x = msq_lpdr_result.data.data[pos].x;
                                info_bk[pos].y = msq_lpdr_result.data.data[pos].y;
                                info_bk[pos].endX = msq_lpdr_result.data.data[pos].endX;
                                info_bk[pos].endY = msq_lpdr_result.data.data[pos].endY;
                                info_bk[pos].score = msq_lpdr_result.data.data[pos].score;  // add. by ariari : 2022.11.16
                                strBKNoFront[pos]   = strNoFront;
                                strBKNoBack[pos]    = strNoBack;
                                strBKHangulFileNameF[pos] = strHangulFileNameF;
                                strBKHangulFileNameB[pos] = strHangulFileNameB;

                                //pos++;
                                //msq_lpdr_result.data.detect_num = pos;

                                //if(pos > 9) break;  // max
                                ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                                QRectF list_rect_left((2*ui->screen->width())/3 + (ui->screen->width()/3)*0.2 + 20,     80 + 20 + pos*80,  (ui->screen->width()/3)*0.25, 48);
                                //QRectF list_rect_right((2*ui->screen->width())/3 + (ui->screen->width()/3)*0.55 + 20,   80 + 20 + pos*80,  (ui->screen->width()/3)*0.35, 48);
                                QRectF list_rect_right((2*ui->screen->width())/3 + (ui->screen->width()/3)*0.45 + 20,   80 + 20 + pos*80,  (ui->screen->width()/3)*0.35, 48);
                                QRectF list_rect_score((2*ui->screen->width())/3 + (ui->screen->width()/3)*0.65 + 20,   80 + 20 + pos*80,  (ui->screen->width()/3)*0.30, 48);


                                //cout << "info_bk[pos].y = " << info_bk[pos].y*live_wnd_height/capHeight << ", red_offset = " << red_offset << endl;

                                //if(msq_lpdr_result.data.data[pos].y > red_offset)     // valid
                                //if(info_bk[pos].y > red_offset)     // valid
                                red_offset = 0;
                                if((info_bk[pos].y*live_wnd_height/capHeight) > red_offset)     // valid
                                {
                                    painter->setPen(QColor(Qt::yellow));
                                    painter->setFont(font_lpr);

                                    painter->drawPixmap(rect_city.x(), 80 + 20 + pos*80, rect_city.width(), 48, QPixmap(strBKHangulFileNameF[pos].c_str()));
                                    painter->drawText(list_rect_left, Qt::AlignHCenter | Qt::AlignVCenter, strBKNoFront[pos].c_str());
                                    painter->drawPixmap(rect_center.x(), 80 + 20 + pos*80, rect_center.width(), 48, QPixmap(strBKHangulFileNameB[pos].c_str()));
                                    painter->drawText(list_rect_right, Qt::AlignHCenter | Qt::AlignVCenter, strBKNoBack[pos].c_str());
                                    //painter->drawText(list_rect_score, Qt::AlignHCenter | Qt::AlignVCenter, QString::number(msq_lpdr_info.data.score));

                                    auto info = QString("%1:%2:%3").arg(info_bk[pos].score).arg(info_bk[pos].endX-info_bk[pos].x).arg(info_bk[pos].endY-info_bk[pos].y);
                                    //painter->drawText(list_rect_score, Qt::AlignHCenter | Qt::AlignVCenter, QString::number(info_bk[pos].score));
                                    painter->drawText(list_rect_score, Qt::AlignHCenter | Qt::AlignVCenter, info);
                                }
                                else
                                {
                                    //painter->setPen(QColor(Qt::red));
                                    painter->setFont(lpd_box);

                                    painter->drawPixmap(rect_city.x(), 80 + 12 + 20 + pos*80, rect_city.width(), 24, QPixmap(strBKHangulFileNameF[pos].c_str()));
                                    painter->drawText(list_rect_left, Qt::AlignHCenter | Qt::AlignVCenter, strBKNoFront[pos].c_str());
                                    painter->drawPixmap(rect_center.x(), 80 + 12 + 20 + pos*80, rect_center.width(), 24, QPixmap(strBKHangulFileNameB[pos].c_str()));
                                    painter->drawText(list_rect_right, Qt::AlignHCenter | Qt::AlignVCenter, strBKNoBack[pos].c_str());
                                    //painter->drawText(list_rect_score, Qt::AlignHCenter | Qt::AlignVCenter, QString::number(msq_lpdr_info.data.score));

                                    auto info = QString("%1:%2:%3").arg(info_bk[pos].score).arg(info_bk[pos].endX-info_bk[pos].x).arg(info_bk[pos].endY-info_bk[pos].y);
                                    //painter->drawText(list_rect_score, Qt::AlignHCenter | Qt::AlignVCenter, QString::number(info_bk[pos].score));
                                    painter->drawText(list_rect_score, Qt::AlignHCenter | Qt::AlignVCenter, info);
                                }

                            #if 0
                                ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                                // backup
                                // claer
                                memset((char*)&info_bk[pos], 0, sizeof(struct lpdr_info));
                                info_bk[pos].x = msq_lpdr_result.data.data[pos].x;
                                info_bk[pos].y = msq_lpdr_result.data.data[pos].y;
                                info_bk[pos].endX = msq_lpdr_result.data.data[pos].endX;
                                info_bk[pos].endY = msq_lpdr_result.data.data[pos].endY;
                                info_bk[pos].score = msq_lpdr_result.data.data[pos].score;
                                strBKNoFront[pos]   = strNoFront;
                                strBKNoBack[pos]    = strNoBack;
                                strBKHangulFileNameF[pos] = strHangulFileNameF;
                                strBKHangulFileNameB[pos] = strHangulFileNameB;
                                //memset((char*)&info_bk[pos].carNo, 0, 32);
                                //memcpy((char*)&info_bk[pos].carNo, (char*)&msq_lpdr_info.data.carNo, strlen(msq_lpdr_info.data.carNo));

                                pos++;
                                msq_lpdr_result.data.detect_num = pos;

                                //if(pos > 9) break;  // max
                                ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                            #endif     
                                pos++;
                                msq_lpdr_result.data.detect_num = pos;
                                if(pos > 9) break;  // max                       

                #endif   //#ifdef DISP_OBJ_LIST                        
                            }
                            //else    // finish
                            //{
                            //    break;
                            //}
                        }
                    }
                }while(Mq_Lpdr_Info->MessageQueueQNum() > 0);
        #else       // do, while...                
                do {
                    // claer
                    memset((char*)&msq_lpdr_info, 0, sizeof(struct lpdr_info));
                    if( Mq_Lpdr_Info->MessageQueueRead((char*)&msq_lpdr_info) > 0 )     // non-blocking mode(IPC_NOWAIT)
                    {
                        

                        // 최대 10(MQ_LPDR_INFO_MAX_QSIZE)
                        int box_sx = msq_lpdr_info.data.x * ratio_w;
                        int box_sy = msq_lpdr_info.data.y * ratio_h;
                        int box_w  = (msq_lpdr_info.data.endX - msq_lpdr_info.data.x) * ratio_w;
                        int box_h  = (msq_lpdr_info.data.endY - msq_lpdr_info.data.y) * ratio_h;

                        painter->setPen(box_pen);
                        painter->drawRect(w1_sx + box_sx, w1_sy + box_sy, box_w, box_h);

            #ifdef DISP_OBJ_LIST
                        GetCarNoInfo(msq_lpdr_info.data.carNo);

                        QRectF list_rect_left((2*ui->screen->width())/3 + (ui->screen->width()/3)*0.2 + 20,     20 + pos*80,        (ui->screen->width()/3)*0.25, 48);
                        //QRectF list_rect_right((2*ui->screen->width())/3 + (ui->screen->width()/3)*0.55 + 20,   20 + pos*80,        (ui->screen->width()/3)*0.35, 48);
                        QRectF list_rect_right((2*ui->screen->width())/3 + (ui->screen->width()/3)*0.45 + 20,   20 + pos*80,        (ui->screen->width()/3)*0.35, 48);
                        QRectF list_rect_score((2*ui->screen->width())/3 + (ui->screen->width()/3)*0.65 + 20,   20 + pos*80 + 48,   (ui->screen->width()/3)*0.30, 48);

                        //if(msq_lpdr_info.data.y > red_offset)     // valid
                        if((msq_lpdr_info.data.y*live_wnd_height/capHeight) > red_offset)     // valid
                        {
                            painter->setPen(QColor(Qt::yellow));
                            painter->setFont(font_lpr);

                            painter->drawPixmap(rect_city.x(), 20 + pos*80, rect_city.width(), 48, QPixmap(strHangulFileNameF.c_str()));
                            painter->drawText(list_rect_left, Qt::AlignHCenter | Qt::AlignVCenter, strNoFront.c_str());
                            painter->drawPixmap(rect_center.x(), 20 + pos*80, rect_center.width(), 48, QPixmap(strHangulFileNameB.c_str()));
                            painter->drawText(list_rect_right, Qt::AlignHCenter | Qt::AlignVCenter, strNoBack.c_str());
                            //painter->drawText(list_rect_score, Qt::AlignHCenter | Qt::AlignVCenter, QString::number(msq_lpdr_info.data.score));
                        }
                        else
                        {
                            //painter->setPen(QColor(Qt::red));     // org//
                            painter->setFont(lpd_box);    // org

                            painter->drawPixmap(rect_city.x(), 12 + 20 + pos*80, rect_city.width(), 24, QPixmap(strHangulFileNameF.c_str()));
                            painter->drawText(list_rect_left, Qt::AlignHCenter | Qt::AlignVCenter, strNoFront.c_str());
                            painter->drawPixmap(rect_center.x(), 12 + 20 + pos*80, rect_center.width(), 24, QPixmap(strHangulFileNameB.c_str()));
                            painter->drawText(list_rect_right, Qt::AlignHCenter | Qt::AlignVCenter, strNoBack.c_str());
                            //painter->drawText(list_rect_score, Qt::AlignHCenter | Qt::AlignVCenter, QString::number(msq_lpdr_info.data.score));
                        }
                        
                        
            #endif  // #ifdef DISP_OBJ_LIST                
                        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                        // backup
                        // claer
                        memset((char*)&info_bk[pos], 0, sizeof(struct lpdr_info));
                        info_bk[pos].x = msq_lpdr_info.data.x;
                        info_bk[pos].y = msq_lpdr_info.data.y;
                        info_bk[pos].endX = msq_lpdr_info.data.endX;
                        info_bk[pos].endY = msq_lpdr_info.data.endY;
                        strBKNoFront[pos]   = strNoFront;
                        strBKNoBack[pos]    = strNoBack;
                        strBKHangulFileNameF[pos] = strHangulFileNameF;
                        strBKHangulFileNameB[pos] = strHangulFileNameB;
                        //memcpy((char*)&info_bk[pos].carNo, (char*)&msq_lpdr_info.data.carNo, strlen(msq_lpdr_info.data.carNo));

                        pos++;
                        //msq_lpdr_result.detect_num = pos;         // rem. by ariari : 2022.11.14
                        msq_lpdr_result.data.detect_num = pos;      // add(mod). by ariari : 2022.11.14

                        if(pos > 9) break;  // max
                        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                    }
                    else 
                    {
                        break;  // finish
                    }
                }while(Mq_Lpdr_Info->MessageQueueQNum() > 0);
        #endif  // for, while..                           
            }
            else    // 이전데이터 디스플레이...
            {
                for(int i=0; i<msq_lpdr_result.data.detect_num; i++)
                {
                    int box_sx = info_bk[i].x * ratio_w;
                    int box_sy = info_bk[i].y * ratio_h;
                    int box_w  = (info_bk[i].endX - info_bk[i].x) * ratio_w;
                    int box_h  = (info_bk[i].endY - info_bk[i].y) * ratio_h;

                    painter->setPen(box_pen);
                    painter->drawRect(w1_sx + box_sx, w1_sy + box_sy, box_w, box_h);

            #ifdef DISP_OBJ_LIST
                    //GetCarNoInfo(info_bk[i].carNo);
                    QRectF list_rect_left((2*ui->screen->width())/3 + (ui->screen->width()/3)*0.2 + 20,     80 + 20 + i*80,  (ui->screen->width()/3)*0.25, 48);
                    QRectF list_rect_right((2*ui->screen->width())/3 + (ui->screen->width()/3)*0.45 + 20,   80 + 20 + i*80,  (ui->screen->width()/3)*0.35, 48);
                    QRectF list_rect_score((2*ui->screen->width())/3 + (ui->screen->width()/3)*0.65 + 20,   80 + 20 + i*80,  (ui->screen->width()/3)*0.30, 48);
                    
                    //if(info_bk[i].y > red_offset)     // valid
                    red_offset = 0;
                    if((info_bk[i].y*live_wnd_height/capHeight) > red_offset)
                    {
                        painter->setPen(QColor(Qt::yellow));  // org
                        painter->setFont(font_lpr);           // org

                        painter->drawPixmap(rect_city.x(), 80 + 20 + i*80, rect_city.width(), 48, QPixmap(strBKHangulFileNameF[i].c_str()));
                        painter->drawText(list_rect_left, Qt::AlignHCenter | Qt::AlignVCenter, strBKNoFront[i].c_str());
                        painter->drawPixmap(rect_center.x(), 80 + 20 + i*80, rect_center.width(), 48, QPixmap(strBKHangulFileNameB[i].c_str()));
                        painter->drawText(list_rect_right, Qt::AlignHCenter | Qt::AlignVCenter, strBKNoBack[i].c_str());

                        auto info = QString("%1:%2:%3").arg(info_bk[i].score).arg(info_bk[i].endX-info_bk[i].x).arg(info_bk[i].endY-info_bk[i].y);
                        //painter->drawText(list_rect_score, Qt::AlignHCenter | Qt::AlignVCenter, QString::number(info_bk[i].score));
                        painter->drawText(list_rect_score, Qt::AlignHCenter | Qt::AlignVCenter, info);
                    }
                    else
                    {
                        painter->setPen(QColor(Qt::red));
                        painter->setFont(lpd_box);

                        painter->drawPixmap(rect_city.x(), 80 + 12 + 20 + i*80, rect_city.width(), 48, QPixmap(strBKHangulFileNameF[i].c_str()));
                        painter->drawText(list_rect_left, Qt::AlignHCenter | Qt::AlignVCenter, strBKNoFront[i].c_str());
                        painter->drawPixmap(rect_center.x(), 80 + 12 + 20 + i*80, rect_center.width(), 48, QPixmap(strBKHangulFileNameB[i].c_str()));
                        painter->drawText(list_rect_right, Qt::AlignHCenter | Qt::AlignVCenter, strBKNoBack[i].c_str());

                        auto info = QString("%1:%2:%3").arg(info_bk[i].score).arg(info_bk[i].endX-info_bk[i].x).arg(info_bk[i].endY-info_bk[i].y);
                        //painter->drawText(list_rect_score, Qt::AlignHCenter | Qt::AlignVCenter, QString::number(info_bk[i].score));
                        painter->drawText(list_rect_score, Qt::AlignHCenter | Qt::AlignVCenter, info);
                    }
            #endif              
                }
            }
        }
    #else   // #if true
        cout << "Sm_Lpdr->SharedMemoryRead" << endl;
        if (Sm_Lpdr->SharedMemoryRead((char *)&msq_lpdr_result) < 0)
        {
            Sm_Lpdr->SharedMemoryInit();
        }
        else 
        {
            cout << "msq_lpdr_result.detect_num  = " << msq_lpdr_result.detect_num  << endl;

            QPen box_pen(Qt::red);
            box_pen.setWidth(4);
            painter->setPen(box_pen);

            //float ratio_w = (float)w1_w/roi_w;
            //float ratio_h = (float)w1_h/roi_h;
            float ratio_w = (float)w1_w/adj_roi_w;  // add. by ariari : 2022.11.16
            float ratio_h = (float)w1_h/adj_roi_h;  // add. by ariari : 2022.11.16

            if (msq_lpdr_result.detect_num > 0)
            {
                
                for(int i=0; msq_lpdr_result.detect_num; i++)
                {
                    cout << "msq_lpdr_result.data[i].x = " << msq_lpdr_result.data[i].x << endl;
                    #if 0
                    int box_sx = msq_lpdr_result.data[i].x * ratio_w;
                    int box_sy = msq_lpdr_result.data[i].y * ratio_h;
                    int box_w  = (msq_lpdr_result.data[i].endX - msq_lpdr_result.data[i].x) * ratio_w;
                    int box_h  = (msq_lpdr_result.data[i].endY - msq_lpdr_result.data[i].y) * ratio_h;

                    cout << "x = " << box_sx << ", y = " << box_sy << ", w = " << box_w << ", h = " << box_h << endl;
                    
                    painter->drawRect(w1_sx + box_sx, w1_sy + box_sy, box_w, box_h);
                    #endif
                }
                
            }

        }
    #endif            

        // add. by ariari : 2022.05.20 - end
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        painter->setPen(QColor(Qt::yellow));
        painter->setFont(font_lpr);
#endif  // #ifdef EN_LIST_DISP        

        // center(lpd)
        //painter->drawPixmap(ui->screen->width()/2, ui->screen->height() - 190, ui->screen->width()/2, 200, QPixmap("/oem/Screen_shot/1.jpg"));
        painter->drawPixmap(ui->screen->width()/3, ui->screen->height() - 190, ui->screen->width()/3, 200, QPixmap("/oem/Screen_shot/1.jpg"));

        // right(string)
        GetCarNoInfo(strCarNo);
        painter->drawPixmap(rect_city.x(), rect_center.y()+76, rect_city.width(), 48, QPixmap(strHangulFileNameF.c_str()));
        painter->drawText(rect_left, Qt::AlignHCenter | Qt::AlignVCenter, strNoFront.c_str());
        painter->drawPixmap(rect_center.x(), rect_center.y()+76, rect_center.width(), 48, QPixmap(strHangulFileNameB.c_str()));
        painter->drawText(rect_right, Qt::AlignHCenter | Qt::AlignVCenter, strNoBack.c_str());

        

        // add. by ariari : begin - 2022.09.19
        WriteDebugInfo(painter);
        // add. by ariari : end - 2022.09.19
        
#if 0   // add. by ariari 
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

            // rem. by ariari : 2022.11.09 : begin
            //debug_rect.setY(debug_rect.y()+30);
            //msg = string_format(" Target Clock Value : %ld[Hz]", st_cam.tarClk);
            //painter->drawText(debug_rect, Qt::AlignTop | Qt::AlignLeft, msg.c_str());
            // rem. by ariari : 2022.11.09 : end

            // add. by ariari : 2022.11.09 : begin
            debug_rect.setY(debug_rect.y()+30);
            msg = string_format(" Target AE Value : %d", st_cam.targetAE);
            painter->drawText(debug_rect, Qt::AlignTop | Qt::AlignLeft, msg.c_str());
            // add. by ariari : 2022.11.09 : end

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
            msg = string_format(" Gain - MIN : %.2f[dB]", st_cam.gainMin);
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
#endif  // // debug message(right/top)

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

        WriteDebugInfo(nullptr);
        
        usleep(5000000);
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

