#####################################################################################################
# Acquisition Makefile
#####################################################################################################

CFLAGS += -std=c++11
CFLAGS += -D_GLIBCXX_USE_CXX11_ABI=1
ifeq ($(wildcard ${OPT_INC}),)
#CXX = g++ ${CFLAGS}
CXX =  /opt/rk3399/rk3399_linux_release_v2.5.1_20210301/buildroot/output/rockchip_rk3399_lpr/host/bin/aarch64-buildroot-linux-gnu-g++ ${CFLAGS}
ODIR  = .obj/build${D}
SDIR  = .
MKDIR = mkdir -p
PLATFORM = $(shell uname)
ifeq ($(PLATFORM),Darwin)
OS = mac
endif
endif

OUTPUTNAME = USBReset${D}
OUTDIR = ./


#_OBJ = main.o


SRCS = $(foreach dir, .. $(SDIR), $(wildcard $(dir)/*.cpp))
SRCS := $(notdir $(SRCS))
_OBJ = $(SRCS:.cpp=.o)
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

# Final binary
${OUTPUTNAME}: ${OBJ}
	${CXX} -o ${OUTPUTNAME} ${OBJ} ${LIB}
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
