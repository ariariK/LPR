#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "threadPreview.h"
#include "ipcs.h"

#include <QMainWindow>
#include <QTimer>
#include <QLabel>
#include <QPixmap>
#include <QPainter>

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
    QString version;
    string msg;


    QFont font_lpr;
    QFont font_dbg;
#ifdef EN_LIST_DISP    
    QFont lpd_box;  // add. by ariari : 2022.05.20
#endif

    QRectF rect_city;
    QRectF rect_left;
    QRectF rect_center;
    QRectF rect_right;

#ifdef EN_LIST_DISP    
    struct lpdr_info{
        //long detect_num;

        char status[32];    // 수배종류
        char carNo[32];     // 차량정보

        // RECT
        int x;              // rect[0]
        int y;              // rect[1]    
        int endX;           // rect[2]
        int endY;           // rect[3]

        // score([0,100]) : add. by ariari : 2022.05.20
        int score;          // score [0,1]->[0,100]
    };
    string strBKNoFront[10];
    string strBKNoBack[10];
    string strBKHangulFileNameF[10];
    string strBKHangulFileNameB[10]; 
    struct lpdr_info info_bk[10];
#endif

    char carNo[32];
    string strNoFront;
    string strNoBack;
    string strHangulF;
    string strHangulB;
    string strHangulFileNameF;
    string strHangulFileNameB;


private slots:
    void Update();
    void Run();

private:
    time_t startTime;

    QTimer *timer;

    Ui::MainWindow *ui;
    ThreadPreview *threadPreview;

    int capWidth;
    int capHeight;
    int screenWidth;
    int screenHeight;

    // add. by ariari : 2022.05.16 - begin
    int draw_en;    // add. by ariari : 2022.07.19
    int roi_sx;
    int roi_sy;
    int roi_w;
    int roi_h;
    // add. by ariari : 2022.05.16 - end

    // add. by ariari : begin - 2022.09.19
    void WriteDebugInfo(QPainter *pPainter);
    // add. by ariari : end - 2022.09.19
    void GetParameters();
    void CheckScreenGeometry();
    void GetCarNoInfo(std::string strCarNo);
    QString GetCPUTemp();
    QString GetRunningTime();

    

    //////////////////////////////////////////////////////////////////////////////////////////////////////	
	// IPC
	//////////////////////////////////////////////////////////////////////////////////////////////////////	
    Ipcs	*Sm_Grab;
    Ipcs	*Sm_Res;
    Ipcs	*Sm_Lpr;
	Ipcs	*Mq_Grab;
    Ipcs	*Sm_Cam;
    // add. by ariari : 2022.05.20
    Ipcs    *Mq_Lpdr_Info;
    Ipcs	*Sm_Lpdr;

    struct grab_data{
		int capWidth;
		int capHeight;
	};
	struct message{
		long msg_type;
		struct grab_data data;
	};

    struct cam_param{
		int capWidth;
		int capHeight;

        // add. by ariri : 2022.11.22 - begin
		int			draw_en;
		int 		roi_sx;
		int 		roi_sy;
		int			roi_w;
		int			roi_h;
		// add. by ariri : 2022.11.22 - end

		uint64_t	capCount;
        double		capFPS;

        int64_t		tarClk;
        int			targetAE;	    // add. by ariari : 2022.11.09
		double		expMax;
		double		expMin;
		double		expCur;
        double		expCurPercent;	// add. by ariari : 2022.11.14
		
		int				dnStatus;
		float			shMax;
		float			shMin;
		float			shCur;
		float			gainMax;
		float			gainMin;
		float			gainCur;
	};

#ifdef EN_LIST_DISP
    // add. by ariari : 2022.05.20 - begin
    struct message_lpdr_info{
        long msg_type;
        struct lpdr_info data;
    };
    struct message_lpdr_info msq_lpdr_info;

    struct message_lpdr_multi{
        long detect_num;
        struct lpdr_info data[10];
    };
    struct lpdr_result{
        long msg_type;
        struct message_lpdr_multi data;
    };
    struct lpdr_result msq_lpdr_result;
    // add. by ariari : 2022.05.20 - end
#endif    

    char buffer[MEM_SIZE_SM];
    struct grab_data st_grab;
	struct message msq;
    struct cam_param st_cam;
	//////////////////////////////////////////////////////////////////////////////////////////////////////	

};

#endif // MAINWINDOW_H
