#####################################################################################################
# Acquisition Makefile
#####################################################################################################

PROJECT_ROOT=./

CFLAGS += -std=c++11
CFLAGS += -D_GLIBCXX_USE_CXX11_ABI=1

#CXX = g++ ${CFLAGS}
CXX =  /opt/rk3399/rk3399_linux_release_v2.5.1_20210301/buildroot/output/rockchip_rk3399_lpr/host/bin/aarch64-buildroot-linux-gnu-g++ ${CFLAGS}
ODIR  = .obj/build${D}
SDIR  = .
MKDIR = mkdir -p
PLATFORM = $(shell uname)

#for release
CFLAGS += -DLOG_LEVEL=6
#for debug
#CFLAGS += -DLOG_LEVEL=7

OUTPUTNAME = GigEGrab${D}
OUTDIR = ./

MVS_LIB = -L ../lib/mindvision/arm64 -lMVSDK -lpthread


SRCS = $(foreach dir, .. $(SDIR), $(wildcard $(dir)/*.cpp))
SRCS := $(notdir $(SRCS))
_OBJ = $(SRCS:.cpp=.o)
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

INC = -I../../include
ifneq ($(OS),mac)
INC += -I../include/mindvision
LIB += -Wl,-Bdynamic ${MVS_LIB}
LIB += -Wl,-rpath-link=../lib/mindvision/arm64
#LIB += -Wl,-rpath=../lib/mindvision/arm64
else
endif

# Final binary
${OUTPUTNAME}: ${OBJ}
	${CXX} -o ${OUTPUTNAME} ${OBJ} ${INC} ${LIB}
#	mv ${OUTPUTNAME} ${OUTDIR}

# Intermediate object files
#@${MKDIR} ./images 
${OBJ}: ${ODIR}/%.o : ${SDIR}/%.cpp
	@${MKDIR} ${ODIR}
	${CXX} ${CFLAGS} ${INC} -Wall -D LINUX -c $< -o $@

# Clean up intermediate objects
clean_obj:
	rm -f ${OBJ}
	@echo "intermediate objects cleaned up!"

# Clean up everything.
clean: clean_obj
	rm -f ${OUTDIR}/${OUTPUTNAME}
	@echo "all cleaned up!"
