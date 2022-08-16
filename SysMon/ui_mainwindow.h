/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 5.12.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QProgressBar>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralwidget;
    QGridLayout *gridLayout;
    QLabel *label_usb_f3_conn;
    QLabel *label_6;
    QLabel *label_usb_r3_conn;
    QProgressBar *progressBar;
    QLabel *label_8;
    QLabel *label_3;
    QLabel *label_9;
    QLabel *label_eth1;
    QLabel *label_usb_f2;
    QLabel *label_4;
    QLabel *label_mac;
    QLabel *label_version;
    QLabel *label_usb_r3;
    QLabel *label;
    QLabel *label_usb_f2_conn;
    QLabel *label_10;
    QLabel *label_eth0;
    QLabel *label_eth0_conn;
    QLabel *label_2;
    QLabel *label_eth1_conn;
    QLabel *label_5;
    QLabel *label_7;
    QLabel *label_11;
    QLabel *label_usb_f3;
    QHBoxLayout *horizontalLayout;
    QLabel *label_SW;
    QLabel *label_CAM;
    QLabel *label_SYS;
    QLabel *label_PWR;
    QLabel *label_12;
    QLabel *label_13;
    QMenuBar *menubar;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QString::fromUtf8("MainWindow"));
        MainWindow->resize(1280, 720);
        QFont font;
        font.setKerning(false);
        MainWindow->setFont(font);
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName(QString::fromUtf8("centralwidget"));
        gridLayout = new QGridLayout(centralwidget);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        label_usb_f3_conn = new QLabel(centralwidget);
        label_usb_f3_conn->setObjectName(QString::fromUtf8("label_usb_f3_conn"));
        QFont font1;
        font1.setFamily(QString::fromUtf8("Arial"));
        font1.setPointSize(16);
        font1.setBold(true);
        font1.setWeight(75);
        font1.setKerning(false);
        label_usb_f3_conn->setFont(font1);
        label_usb_f3_conn->setFrameShape(QFrame::Box);
        label_usb_f3_conn->setAlignment(Qt::AlignCenter);

        gridLayout->addWidget(label_usb_f3_conn, 7, 2, 1, 1);

        label_6 = new QLabel(centralwidget);
        label_6->setObjectName(QString::fromUtf8("label_6"));
        label_6->setFont(font1);
        label_6->setFrameShape(QFrame::Box);

        gridLayout->addWidget(label_6, 5, 0, 1, 1);

        label_usb_r3_conn = new QLabel(centralwidget);
        label_usb_r3_conn->setObjectName(QString::fromUtf8("label_usb_r3_conn"));
        label_usb_r3_conn->setFont(font1);
        label_usb_r3_conn->setFrameShape(QFrame::Box);
        label_usb_r3_conn->setAlignment(Qt::AlignCenter);

        gridLayout->addWidget(label_usb_r3_conn, 2, 0, 1, 1);

        progressBar = new QProgressBar(centralwidget);
        progressBar->setObjectName(QString::fromUtf8("progressBar"));
        QFont font2;
        font2.setFamily(QString::fromUtf8("Arial"));
        font2.setPointSize(20);
        font2.setBold(true);
        font2.setWeight(75);
        font2.setKerning(false);
        progressBar->setFont(font2);
        progressBar->setMinimum(0);
        progressBar->setMaximum(100);
        progressBar->setValue(100);
        progressBar->setAlignment(Qt::AlignCenter);

        gridLayout->addWidget(progressBar, 19, 0, 1, 3);

        label_8 = new QLabel(centralwidget);
        label_8->setObjectName(QString::fromUtf8("label_8"));
        label_8->setFont(font1);
        label_8->setAlignment(Qt::AlignCenter);

        gridLayout->addWidget(label_8, 3, 1, 1, 1);

        label_3 = new QLabel(centralwidget);
        label_3->setObjectName(QString::fromUtf8("label_3"));
        label_3->setFont(font1);
        label_3->setFrameShape(QFrame::Box);

        gridLayout->addWidget(label_3, 7, 0, 1, 1);

        label_9 = new QLabel(centralwidget);
        label_9->setObjectName(QString::fromUtf8("label_9"));
        label_9->setFont(font1);
        label_9->setAlignment(Qt::AlignCenter);

        gridLayout->addWidget(label_9, 3, 2, 1, 1);

        label_eth1 = new QLabel(centralwidget);
        label_eth1->setObjectName(QString::fromUtf8("label_eth1"));
        label_eth1->setFont(font1);
        label_eth1->setFrameShape(QFrame::Box);
        label_eth1->setAlignment(Qt::AlignCenter);

        gridLayout->addWidget(label_eth1, 5, 1, 1, 1);

        label_usb_f2 = new QLabel(centralwidget);
        label_usb_f2->setObjectName(QString::fromUtf8("label_usb_f2"));
        label_usb_f2->setFont(font1);
        label_usb_f2->setFrameShape(QFrame::Box);
        label_usb_f2->setAlignment(Qt::AlignCenter);

        gridLayout->addWidget(label_usb_f2, 6, 1, 1, 1);

        label_4 = new QLabel(centralwidget);
        label_4->setObjectName(QString::fromUtf8("label_4"));
        label_4->setFont(font1);
        label_4->setFrameShape(QFrame::Box);

        gridLayout->addWidget(label_4, 0, 0, 1, 1);

        label_mac = new QLabel(centralwidget);
        label_mac->setObjectName(QString::fromUtf8("label_mac"));
        label_mac->setFont(font1);
        label_mac->setFrameShape(QFrame::Box);
        label_mac->setAlignment(Qt::AlignCenter);

        gridLayout->addWidget(label_mac, 8, 1, 1, 2);

        label_version = new QLabel(centralwidget);
        label_version->setObjectName(QString::fromUtf8("label_version"));
        QFont font3;
        font3.setFamily(QString::fromUtf8("Arial"));
        font3.setPointSize(16);
        font3.setBold(true);
        font3.setItalic(false);
        font3.setWeight(75);
        font3.setKerning(false);
        label_version->setFont(font3);
        label_version->setAlignment(Qt::AlignBottom|Qt::AlignRight|Qt::AlignTrailing);

        gridLayout->addWidget(label_version, 0, 2, 1, 1);

        label_usb_r3 = new QLabel(centralwidget);
        label_usb_r3->setObjectName(QString::fromUtf8("label_usb_r3"));
        label_usb_r3->setFont(font1);
        label_usb_r3->setFrameShape(QFrame::Box);
        label_usb_r3->setAlignment(Qt::AlignCenter);

        gridLayout->addWidget(label_usb_r3, 1, 0, 1, 1);

        label = new QLabel(centralwidget);
        label->setObjectName(QString::fromUtf8("label"));
        label->setFont(font1);
        label->setFrameShape(QFrame::Box);

        gridLayout->addWidget(label, 4, 0, 1, 1);

        label_usb_f2_conn = new QLabel(centralwidget);
        label_usb_f2_conn->setObjectName(QString::fromUtf8("label_usb_f2_conn"));
        label_usb_f2_conn->setFont(font1);
        label_usb_f2_conn->setFrameShape(QFrame::Box);
        label_usb_f2_conn->setAlignment(Qt::AlignCenter);

        gridLayout->addWidget(label_usb_f2_conn, 6, 2, 1, 1);

        label_10 = new QLabel(centralwidget);
        label_10->setObjectName(QString::fromUtf8("label_10"));
        label_10->setFont(font1);
        label_10->setFrameShape(QFrame::Box);

        gridLayout->addWidget(label_10, 8, 0, 1, 1);

        label_eth0 = new QLabel(centralwidget);
        label_eth0->setObjectName(QString::fromUtf8("label_eth0"));
        label_eth0->setFont(font1);
        label_eth0->setFrameShape(QFrame::Box);
        label_eth0->setAlignment(Qt::AlignCenter);

        gridLayout->addWidget(label_eth0, 4, 1, 1, 1);

        label_eth0_conn = new QLabel(centralwidget);
        label_eth0_conn->setObjectName(QString::fromUtf8("label_eth0_conn"));
        label_eth0_conn->setFont(font1);
        label_eth0_conn->setFrameShape(QFrame::Box);
        label_eth0_conn->setAlignment(Qt::AlignCenter);

        gridLayout->addWidget(label_eth0_conn, 4, 2, 1, 1);

        label_2 = new QLabel(centralwidget);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        label_2->setFont(font1);
        label_2->setFrameShape(QFrame::Box);

        gridLayout->addWidget(label_2, 6, 0, 1, 1);

        label_eth1_conn = new QLabel(centralwidget);
        label_eth1_conn->setObjectName(QString::fromUtf8("label_eth1_conn"));
        label_eth1_conn->setFont(font1);
        label_eth1_conn->setFrameShape(QFrame::Box);
        label_eth1_conn->setAlignment(Qt::AlignCenter);

        gridLayout->addWidget(label_eth1_conn, 5, 2, 1, 1);

        label_5 = new QLabel(centralwidget);
        label_5->setObjectName(QString::fromUtf8("label_5"));
        QFont font4;
        font4.setFamily(QString::fromUtf8("Arial"));
        font4.setPointSize(24);
        font4.setBold(true);
        font4.setItalic(false);
        font4.setWeight(75);
        font4.setKerning(false);
        label_5->setFont(font4);
        label_5->setAlignment(Qt::AlignCenter);

        gridLayout->addWidget(label_5, 0, 1, 1, 1);

        label_7 = new QLabel(centralwidget);
        label_7->setObjectName(QString::fromUtf8("label_7"));
        label_7->setFont(font1);
        label_7->setAlignment(Qt::AlignCenter);

        gridLayout->addWidget(label_7, 3, 0, 1, 1);

        label_11 = new QLabel(centralwidget);
        label_11->setObjectName(QString::fromUtf8("label_11"));
        label_11->setFont(font1);
        label_11->setFrameShape(QFrame::Box);

        gridLayout->addWidget(label_11, 16, 0, 1, 1);

        label_usb_f3 = new QLabel(centralwidget);
        label_usb_f3->setObjectName(QString::fromUtf8("label_usb_f3"));
        label_usb_f3->setFont(font1);
        label_usb_f3->setFrameShape(QFrame::Box);
        label_usb_f3->setAlignment(Qt::AlignCenter);

        gridLayout->addWidget(label_usb_f3, 7, 1, 1, 1);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        label_SW = new QLabel(centralwidget);
        label_SW->setObjectName(QString::fromUtf8("label_SW"));
        label_SW->setFont(font1);
        label_SW->setFrameShape(QFrame::Box);
        label_SW->setAlignment(Qt::AlignCenter);

        horizontalLayout->addWidget(label_SW);

        label_CAM = new QLabel(centralwidget);
        label_CAM->setObjectName(QString::fromUtf8("label_CAM"));
        label_CAM->setFont(font1);
        label_CAM->setFrameShape(QFrame::Box);
        label_CAM->setAlignment(Qt::AlignCenter);

        horizontalLayout->addWidget(label_CAM);

        label_SYS = new QLabel(centralwidget);
        label_SYS->setObjectName(QString::fromUtf8("label_SYS"));
        label_SYS->setFont(font1);
        label_SYS->setFrameShape(QFrame::Box);
        label_SYS->setAlignment(Qt::AlignCenter);

        horizontalLayout->addWidget(label_SYS);

        label_PWR = new QLabel(centralwidget);
        label_PWR->setObjectName(QString::fromUtf8("label_PWR"));
        label_PWR->setFont(font1);
        label_PWR->setFrameShape(QFrame::Box);
        label_PWR->setAlignment(Qt::AlignCenter);

        horizontalLayout->addWidget(label_PWR);


        gridLayout->addLayout(horizontalLayout, 16, 1, 1, 2);

        label_12 = new QLabel(centralwidget);
        label_12->setObjectName(QString::fromUtf8("label_12"));
        label_12->setFont(font1);
        label_12->setFrameShape(QFrame::Box);

        gridLayout->addWidget(label_12, 17, 0, 1, 1);

        label_13 = new QLabel(centralwidget);
        label_13->setObjectName(QString::fromUtf8("label_13"));
        label_13->setFont(font1);
        label_13->setFrameShape(QFrame::Box);
        label_13->setAlignment(Qt::AlignCenter);

        gridLayout->addWidget(label_13, 17, 1, 1, 2);

        MainWindow->setCentralWidget(centralwidget);
        menubar = new QMenuBar(MainWindow);
        menubar->setObjectName(QString::fromUtf8("menubar"));
        menubar->setGeometry(QRect(0, 0, 1280, 20));
        MainWindow->setMenuBar(menubar);
        statusbar = new QStatusBar(MainWindow);
        statusbar->setObjectName(QString::fromUtf8("statusbar"));
        MainWindow->setStatusBar(statusbar);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QApplication::translate("MainWindow", "MainWindow", nullptr));
        label_usb_f3_conn->setText(QString());
        label_6->setText(QApplication::translate("MainWindow", "Checking Ethernet Port 2", nullptr));
        label_usb_r3_conn->setText(QString());
        label_8->setText(QApplication::translate("MainWindow", "D/D Loading", nullptr));
        label_3->setText(QApplication::translate("MainWindow", "Checking USB Port 2 (FRONT 3.0)", nullptr));
        label_9->setText(QApplication::translate("MainWindow", "Action", nullptr));
        label_eth1->setText(QString());
        label_usb_f2->setText(QString());
        label_4->setText(QApplication::translate("MainWindow", "Checking USB Port 3 (REAR 3.0)", nullptr));
        label_mac->setText(QString());
        label_version->setText(QApplication::translate("MainWindow", "Ver.", nullptr));
        label_usb_r3->setText(QString());
        label->setText(QApplication::translate("MainWindow", "Checking Ethernet Port 1", nullptr));
        label_usb_f2_conn->setText(QString());
        label_10->setText(QApplication::translate("MainWindow", "MAC ADDRESS", nullptr));
        label_eth0->setText(QString());
        label_eth0_conn->setText(QString());
        label_2->setText(QApplication::translate("MainWindow", "Checking USB Port 1 (FRONT 2.0)", nullptr));
        label_eth1_conn->setText(QString());
        label_5->setText(QApplication::translate("MainWindow", "SYSTEM HEALTH CHECK", nullptr));
        label_7->setText(QApplication::translate("MainWindow", "Check List", nullptr));
        label_11->setText(QApplication::translate("MainWindow", "LED STATUS", nullptr));
        label_usb_f3->setText(QString());
        label_SW->setText(QApplication::translate("MainWindow", "SWITCH", nullptr));
        label_CAM->setText(QApplication::translate("MainWindow", "CAMERA", nullptr));
        label_SYS->setText(QApplication::translate("MainWindow", "SYSTEM", nullptr));
        label_PWR->setText(QApplication::translate("MainWindow", "POWER", nullptr));
        label_12->setText(QApplication::translate("MainWindow", "RESULT", nullptr));
        label_13->setText(QApplication::translate("MainWindow", "NG", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
