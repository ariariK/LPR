cmake_minimum_required(VERSION 3.16)
project(Comm)
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

#define 추가
# for release version
add_definitions(-DLOG_LEVEL=6)
# for debug version
#add_definitions(-DLOG_LEVEL=7)

########################################################################################################################


########################################################################################################################
# Header
set(INC_DIR 
	./
)
include_directories(${INC_DIR})

# Library


########################################################################################################################

########################################################################################################################
add_executable(Comm
		src/ipcs.cpp
		src/socket.cpp
		src/usage.cpp
		src/main.cpp
)

target_link_libraries(Comm dl pthread)
########################################################################################################################

########################################################################################################################
########################################################################################################################
