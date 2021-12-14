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
    QRectF rect_city;
    QRectF rect_left;
    QRectF rect_center;
    QRectF rect_right;

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

    char carNo[32];
    string strNoFront;
    string strNoBack;
    string strHangulF;
    string strHangulB;
    string strHangulFileNameF;
    string strHangulFileNameB;

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

		uint64_t	capCount;
        double		capFPS;

        int64_t		tarClk;
		double		expMax;
		double		expMin;
		double		expCur;
		
		int				dnStatus;
		float			shMax;
		float			shMin;
		float			shCur;
		float			gainMax;
		float			gainMin;
		float			gainCur;
	};

    char buffer[MEM_SIZE_SM];
    struct grab_data st_grab;
	struct message msq;
    struct cam_param st_cam;
	//////////////////////////////////////////////////////////////////////////////////////////////////////	

};

#endif // MAINWINDOW_H
