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
    QLabel *label_2;
    QLabel *label_usb_f2;
    QLabel *label_4;
    QLabel *label_3;
    QLabel *label_8;
    QLabel *label_usb_f3;
    QLabel *label_5;
    QLabel *label;
    QLabel *label_eth1;
    QLabel *label_usb_r3_conn;
    QLabel *label_usb_r3;
    QLabel *label_usb_f2_conn;
    QLabel *label_10;
    QLabel *label_eth0;
    QLabel *label_eth1_conn;
    QLabel *label_9;
    QLabel *label_6;
    QLabel *label_7;
    QLabel *label_mac;
    QProgressBar *progressBar;
    QLabel *label_eth0_conn;
    QLabel *label_usb_f3_conn;
    QMenuBar *menubar;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QString::fromUtf8("MainWindow"));
        MainWindow->resize(1280, 720);
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName(QString::fromUtf8("centralwidget"));
        gridLayout = new QGridLayout(centralwidget);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        label_2 = new QLabel(centralwidget);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        QFont font;
        font.setPointSize(16);
        font.setBold(true);
        font.setWeight(75);
        label_2->setFont(font);
        label_2->setFrameShape(QFrame::Box);

        gridLayout->addWidget(label_2, 5, 0, 1, 1);

        label_usb_f2 = new QLabel(centralwidget);
        label_usb_f2->setObjectName(QString::fromUtf8("label_usb_f2"));
        label_usb_f2->setFont(font);
        label_usb_f2->setFrameShape(QFrame::Box);
        label_usb_f2->setAlignment(Qt::AlignCenter);

        gridLayout->addWidget(label_usb_f2, 5, 1, 1, 1);

        label_4 = new QLabel(centralwidget);
        label_4->setObjectName(QString::fromUtf8("label_4"));
        label_4->setFont(font);
        label_4->setFrameShape(QFrame::Box);

        gridLayout->addWidget(label_4, 7, 0, 1, 1);

        label_3 = new QLabel(centralwidget);
        label_3->setObjectName(QString::fromUtf8("label_3"));
        label_3->setFont(font);
        label_3->setFrameShape(QFrame::Box);

        gridLayout->addWidget(label_3, 6, 0, 1, 1);

        label_8 = new QLabel(centralwidget);
        label_8->setObjectName(QString::fromUtf8("label_8"));
        label_8->setFont(font);
        label_8->setAlignment(Qt::AlignCenter);

        gridLayout->addWidget(label_8, 1, 1, 1, 1);

        label_usb_f3 = new QLabel(centralwidget);
        label_usb_f3->setObjectName(QString::fromUtf8("label_usb_f3"));
        label_usb_f3->setFont(font);
        label_usb_f3->setFrameShape(QFrame::Box);
        label_usb_f3->setAlignment(Qt::AlignCenter);

        gridLayout->addWidget(label_usb_f3, 6, 1, 1, 1);

        label_5 = new QLabel(centralwidget);
        label_5->setObjectName(QString::fromUtf8("label_5"));
        QFont font1;
        font1.setPointSize(24);
        font1.setBold(true);
        font1.setItalic(false);
        font1.setWeight(75);
        font1.setKerning(false);
        label_5->setFont(font1);
        label_5->setAlignment(Qt::AlignCenter);

        gridLayout->addWidget(label_5, 0, 0, 1, 3);

        label = new QLabel(centralwidget);
        label->setObjectName(QString::fromUtf8("label"));
        label->setFont(font);
        label->setFrameShape(QFrame::Box);

        gridLayout->addWidget(label, 3, 0, 1, 1);

        label_eth1 = new QLabel(centralwidget);
        label_eth1->setObjectName(QString::fromUtf8("label_eth1"));
        label_eth1->setFont(font);
        label_eth1->setFrameShape(QFrame::Box);
        label_eth1->setAlignment(Qt::AlignCenter);

        gridLayout->addWidget(label_eth1, 4, 1, 1, 1);

        label_usb_r3_conn = new QLabel(centralwidget);
        label_usb_r3_conn->setObjectName(QString::fromUtf8("label_usb_r3_conn"));
        label_usb_r3_conn->setFont(font);
        label_usb_r3_conn->setFrameShape(QFrame::Box);
        label_usb_r3_conn->setAlignment(Qt::AlignCenter);

        gridLayout->addWidget(label_usb_r3_conn, 7, 2, 1, 1);

        label_usb_r3 = new QLabel(centralwidget);
        label_usb_r3->setObjectName(QString::fromUtf8("label_usb_r3"));
        label_usb_r3->setFont(font);
        label_usb_r3->setFrameShape(QFrame::Box);
        label_usb_r3->setAlignment(Qt::AlignCenter);

        gridLayout->addWidget(label_usb_r3, 7, 1, 1, 1);

        label_usb_f2_conn = new QLabel(centralwidget);
        label_usb_f2_conn->setObjectName(QString::fromUtf8("label_usb_f2_conn"));
        label_usb_f2_conn->setFont(font);
        label_usb_f2_conn->setFrameShape(QFrame::Box);
        label_usb_f2_conn->setAlignment(Qt::AlignCenter);

        gridLayout->addWidget(label_usb_f2_conn, 5, 2, 1, 1);

        label_10 = new QLabel(centralwidget);
        label_10->setObjectName(QString::fromUtf8("label_10"));
        label_10->setFont(font);
        label_10->setFrameShape(QFrame::Box);

        gridLayout->addWidget(label_10, 8, 0, 1, 1);

        label_eth0 = new QLabel(centralwidget);
        label_eth0->setObjectName(QString::fromUtf8("label_eth0"));
        label_eth0->setFont(font);
        label_eth0->setFrameShape(QFrame::Box);
        label_eth0->setAlignment(Qt::AlignCenter);

        gridLayout->addWidget(label_eth0, 3, 1, 1, 1);

        label_eth1_conn = new QLabel(centralwidget);
        label_eth1_conn->setObjectName(QString::fromUtf8("label_eth1_conn"));
        label_eth1_conn->setFont(font);
        label_eth1_conn->setFrameShape(QFrame::Box);
        label_eth1_conn->setAlignment(Qt::AlignCenter);

        gridLayout->addWidget(label_eth1_conn, 4, 2, 1, 1);

        label_9 = new QLabel(centralwidget);
        label_9->setObjectName(QString::fromUtf8("label_9"));
        label_9->setFont(font);
        label_9->setAlignment(Qt::AlignCenter);

        gridLayout->addWidget(label_9, 1, 2, 1, 1);

        label_6 = new QLabel(centralwidget);
        label_6->setObjectName(QString::fromUtf8("label_6"));
        label_6->setFont(font);
        label_6->setFrameShape(QFrame::Box);

        gridLayout->addWidget(label_6, 4, 0, 1, 1);

        label_7 = new QLabel(centralwidget);
        label_7->setObjectName(QString::fromUtf8("label_7"));
        label_7->setFont(font);
        label_7->setAlignment(Qt::AlignCenter);

        gridLayout->addWidget(label_7, 1, 0, 1, 1);

        label_mac = new QLabel(centralwidget);
        label_mac->setObjectName(QString::fromUtf8("label_mac"));
        label_mac->setFont(font);
        label_mac->setFrameShape(QFrame::Box);
        label_mac->setAlignment(Qt::AlignCenter);

        gridLayout->addWidget(label_mac, 8, 1, 1, 2);

        progressBar = new QProgressBar(centralwidget);
        progressBar->setObjectName(QString::fromUtf8("progressBar"));
        QFont font2;
        font2.setPointSize(20);
        font2.setBold(true);
        font2.setWeight(75);
        progressBar->setFont(font2);
        progressBar->setMinimum(0);
        progressBar->setMaximum(100);
        progressBar->setValue(100);
        progressBar->setAlignment(Qt::AlignCenter);

        gridLayout->addWidget(progressBar, 9, 0, 1, 3);

        label_eth0_conn = new QLabel(centralwidget);
        label_eth0_conn->setObjectName(QString::fromUtf8("label_eth0_conn"));
        label_eth0_conn->setFont(font);
        label_eth0_conn->setFrameShape(QFrame::Box);
        label_eth0_conn->setAlignment(Qt::AlignCenter);

        gridLayout->addWidget(label_eth0_conn, 3, 2, 1, 1);

        label_usb_f3_conn = new QLabel(centralwidget);
        label_usb_f3_conn->setObjectName(QString::fromUtf8("label_usb_f3_conn"));
        label_usb_f3_conn->setFont(font);
        label_usb_f3_conn->setFrameShape(QFrame::Box);
        label_usb_f3_conn->setAlignment(Qt::AlignCenter);

        gridLayout->addWidget(label_usb_f3_conn, 6, 2, 1, 1);

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
        label_2->setText(QApplication::translate("MainWindow", "Checking USB Port 1 (FRONT 2.0)", nullptr));
        label_usb_f2->setText(QString());
        label_4->setText(QApplication::translate("MainWindow", "Checking USB Port 3 (REAR 3.0)", nullptr));
        label_3->setText(QApplication::translate("MainWindow", "Checking USB Port 2 (FRONT 3.0)", nullptr));
        label_8->setText(QApplication::translate("MainWindow", "D/D Loading", nullptr));
        label_usb_f3->setText(QString());
        label_5->setText(QApplication::translate("MainWindow", "SYSTEM HEALTH CHECK", nullptr));
        label->setText(QApplication::translate("MainWindow", "Checking Ethernet Port 1", nullptr));
        label_eth1->setText(QString());
        label_usb_r3_conn->setText(QString());
        label_usb_r3->setText(QString());
        label_usb_f2_conn->setText(QString());
        label_10->setText(QApplication::translate("MainWindow", "MAC ADDRESS", nullptr));
        label_eth0->setText(QString());
        label_eth1_conn->setText(QString());
        label_9->setText(QApplication::translate("MainWindow", "Action", nullptr));
        label_6->setText(QApplication::translate("MainWindow", "Checking Ethernet Port 2", nullptr));
        label_7->setText(QApplication::translate("MainWindow", "Check List", nullptr));
        label_mac->setText(QString());
        label_eth0_conn->setText(QString());
        label_usb_f3_conn->setText(QString());
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
