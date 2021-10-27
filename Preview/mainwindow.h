#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "threadPreview.h"

#include <QMainWindow>
#include <QTimer>
#include <QLabel>

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
    

private slots:
    void Update();
    void Run();

private:
    QTimer *timer;

    Ui::MainWindow *ui;
    ThreadPreview *threadPreview;

    int capWidth;
    int capHeight;
    void GetParameters();

    //////////////////////////////////////////////////////////////////////////////////////////////////////	
	// IPC : Shared Memory
	//////////////////////////////////////////////////////////////////////////////////////////////////////	
    int shmid;
    int SharedMemoryInit();
    int SharedMemoryRead(char *sMemory);
    //////////////////////////////////////////////////////////////////////////////////////////////////////	

    //////////////////////////////////////////////////////////////////////////////////////////////////////	
	// IPC : Message Queue
	//////////////////////////////////////////////////////////////////////////////////////////////////////	
	int msqid;
	struct real_data{
		int capWidth;
		int capHeight;
	};
	struct message{
		long msg_type;
		struct real_data data;
	};
	struct message msq;
	int MessageQueueInit();
	int MessageQueueRead();
	int MessageQueueFree();
	//////////////////////////////////////////////////////////////////////////////////////////////////////	

};

#endif // MAINWINDOW_H
