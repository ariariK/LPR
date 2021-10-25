cmake_minimum_required(VERSION 3.16)
project(Inference)
set(CMAKE_CXX_STANDARD 14)

########################################################################################################################
########################################################################################################################
set(armcc "/opt/rk3399/rk3399_linux_release_v2.5.1_20210301/buildroot/output/rockchip_rk3399_lpr/host/bin/aarch64-buildroot-linux-gnu-c++")
set(armgg "/opt/rk3399/rk3399_linux_release_v2.5.1_20210301/buildroot/output/rockchip_rk3399_lpr/host/bin/aarch64-buildroot-linux-gnu-g++")
set(armlink "/opt/rk3399/rk3399_linux_release_v2.5.1_20210301/buildroot/output/rockchip_rk3399_lpr/host/bin/aarch64-buildroot-linux-gnu-ld")

SET(CMAKE_C_COMPILER ${armcc})
SET(CMAKE_CXX_COMPILER ${armgg})
SET(CMAKE_LINKER ${armlink})
SET(CMAKE_C_LINK_EXECUTABLE ${armlink})
########################################################################################################################


########################################################################################################################
########################################################################################################################
set(INC_DIR
        ../include
        /opt/tensorflow_src
        /opt/tensorflow_src/tensorflow/lite/tools/make/downloads/flatbuffers/include
        )

set(OPENCV_INC_DIR
        /opt/rk3399/rk3399_linux_release_v2.5.1_20210301/buildroot/output/rockchip_rk3399_lpr/host/aarch64-buildroot-linux-gnu/sysroot/usr/include)
include_directories(${INC_DIR} ${OPENCV_INC_DIR})

# libs
set(LINK_DIR /opt/tflite_build)
set(OPENCV_LINK_DIR
        /opt/rk3399/rk3399_linux_release_v2.5.1_20210301/buildroot/output/rockchip_rk3399_lpr/host/aarch64-buildroot-linux-gnu/sysroot/usr/lib
)
link_directories(${LINK_DIR} ${OPENCV_LINK_DIR})

set(OPENCV_LIBS
                opencv_core
                opencv_highgui
                opencv_imgproc
                opencv_imgcodecs
                opencv_features2d
                opencv_flann
                opencv_ml
                opencv_videoio
                opencv_dnn
)

set(TF_LIBS
        libtensorflow-lite.a
)
########################################################################################################################

########################################################################################################################
########################################################################################################################
add_executable( Inference
	#                l_ocr.h
	#        l_detect.h
	#        utils.h
                src/lpr_demo/l_ocr.cpp
                src/lpr_demo/l_detect.cpp
                src/lpr_demo/utils.cpp
                src/main.cpp
                )
target_link_libraries(Inference dl pthread ${OPENCV_LIBS} ${TF_LIBS})
########################################################################################################################
