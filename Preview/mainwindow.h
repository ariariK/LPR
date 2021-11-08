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
    QString msg;
    

private slots:
    void Update();
    void Run();

private:
    QTimer *timer;

    Ui::MainWindow *ui;
    ThreadPreview *threadPreview;

    int capWidth;
    int capHeight;
    int screenWidth;
    int screenHeight;
    void GetParameters();

    //////////////////////////////////////////////////////////////////////////////////////////////////////	
	// IPC
	//////////////////////////////////////////////////////////////////////////////////////////////////////	
    Ipcs	*Sm_Grab;
    Ipcs	*Sm_Res;
	Ipcs	*Mq_Grab;

    struct grab_data{
		int capWidth;
		int capHeight;
	};
	struct message{
		long msg_type;
		struct grab_data data;
	};

    char buffer[MEM_SIZE_SM];
    struct grab_data st_grab;
	struct message msq;
	//////////////////////////////////////////////////////////////////////////////////////////////////////	

};

#endif // MAINWINDOW_H
