QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    threadPreview.cpp \
    ipcs.cpp

HEADERS += \
    mainwindow.h \
    threadPreview.h \
    ipcs.h

FORMS += \
    mainwindow.ui

win32{
INCLUDEPATH += D:\opencv\build\install\include\
LIBS += D:\opencv\build\install\x64\mingw\bin\libopencv_core3412.dll \
        D:\opencv\build\install\x64\mingw\bin\libopencv_highgui3412.dll \
        D:\opencv\build\install\x64\mingw\bin\libopencv_imgproc3412.dll \
        D:\opencv\build\install\x64\mingw\bin\libopencv_calib3d3412.dll \
        D:\opencv\build\install\x64\mingw\bin\libopencv_features2d3412.dll \
        D:\opencv\build\install\x64\mingw\bin\libopencv_imgcodecs3412.dll
}

unix{
INCLUDEPATH += /opt/rk3399/rk3399_linux_release_v2.5.1_20210301/buildroot/output/rockchip_rk3399_lpr/host/aarch64-buildroot-linux-gnu/sysroot/usr/include
LIBS += -L/opt/rk3399/rk3399_linux_release_v2.5.1_20210301/buildroot/output/rockchip_rk3399_lpr/host/aarch64-buildroot-linux-gnu/sysroot/usr/lib \
        -lopencv_core \
        -lopencv_highgui \
        -lopencv_imgproc \
        -lopencv_imgcodecs \
        -lopencv_features2d \
        -lopencv_flann \
        -lopencv_ml \
        -lopencv_videoio


INCLUDEPATH += ../include/spinnaker
LIBS += -L ../lib/spinnaker \
	-lGenApi_gcc540_v3_0 \
	-lLog_gcc540_v3_0 \
	-lMathParser_gcc540_v3_0 \
	-lNodeMapData_gcc540_v3_0 \
	-lGCBase_gcc540_v3_0 \
	-lSpinUpdate \
	-lSpinVideo \
	-lSpinVideo_C \
	-lSpinnaker \
	-lSpinnaker_C \
	-lgomp
}

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
