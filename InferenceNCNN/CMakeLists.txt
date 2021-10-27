cmake_minimum_required(VERSION 3.16)
project(InferenceNCNN)
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
        ../include/ncnn
        /opt/tensorflow_src
        /opt/tensorflow_src/tensorflow/lite/tools/make/downloads/flatbuffers/include
)

set(OPENCV_INC_DIR
        /opt/rk3399/rk3399_linux_release_v2.5.1_20210301/buildroot/output/rockchip_rk3399_lpr/host/aarch64-buildroot-linux-gnu/sysroot/usr/include)
include_directories(${INC_DIR} ${OPENCV_INC_DIR})

# libs
set(LPR_LINK_DIR ../lib/lpr)
set(TF_LINK_DIR /opt/tflite_build)
set(OPENCV_LINK_DIR
        /opt/rk3399/rk3399_linux_release_v2.5.1_20210301/buildroot/output/rockchip_rk3399_lpr/host/aarch64-buildroot-linux-gnu/sysroot/usr/lib
)
link_directories(${LPR_LINK_DIR} ${TF_LINK_DIR} ${OPENCV_LINK_DIR})
file(GLOB LIBS_LPR "${LPR_LINK_DIR}/lib*")
file(GLOB LIBS_TF "${TF_LINK_DIR}/libtensorflow-lite.a")
file(GLOB LIBS_OPENCV "${OPENCV_LINK_DIR}/libopencv*")

# flags
#set(CMAKE_CXX_FLAGS "-fopenmp")

########################################################################################################################
########################################################################################################################
add_executable( InferenceNCNN
                src/lpr_demo/l_ocr.cpp
                src/lpr_demo/l_detect.cpp
                src/lpr_demo/utils.cpp
                src/main.cpp
                )
target_link_libraries(InferenceNCNN dl pthread ${LIBS_LPR} ${LIBS_TF} ${LIBS_OPENCV})
########################################################################################################################
