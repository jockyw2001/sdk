# Hisilicon Hi3516 sample Makefile

include ../Makefile.param
#ifeq ($(SAMPLE_PARAM_FILE), )
#     SAMPLE_PARAM_FILE:=../Makefile.param
#     include $(SAMPLE_PARAM_FILE)
#endif

# target source
SRC  := $(wildcard *.c) 
OBJ  := $(SRC:%.c=%.o)
SRC  := $(wildcard *.cpp)
OBJ  := $(SRC:%.cpp=%.o)

RTSP_SRC := $(wildcard src/*.cpp)
RTSP_OBJ := $(RTSP_SRC:%.cpp=%.o)

CROSS_COMPILE=arm-hisiv300-linux-
CXX=$(CROSS_COMPILE)g++

TARGET := $(OBJ:%.o=%)
.PHONY : clean all

all: $(TARGET)

MPI_LIBS := $(REL_LIB)/libmpi.a

CFLAGS  += -I./include/UsageEnvironment
CFLAGS  += -I./include/groupsock
CFLAGS  += -I./include/liveMedia
CFLAGS  += -I./include/BasicUsageEnvironment
CFLAGS  += -I./src

LIBS += -L./lib
LIBS += -lliveMedia -lgroupsock -lBasicUsageEnvironment -lUsageEnvironment

.c.o:
	$(CC) -c $(CFLAGS) $< -o $@

.cpp.o:
	$(CXX) -c $(CFLAGS) $< -o $@

$(TARGET):%:%.o $(COMM_OBJ) $(RTSP_OBJ)
	$(CXX) $(CFLAGS) -lpthread -lm -o $@ $^ $(MPI_LIBS) $(AUDIO_LIBA) $(SENSOR_LIBS) $(LIBS)

clean:
	@rm -f $(TARGET)
	@rm -f $(OBJ)
	@rm -f $(COMM_OBJ)

cleanstream:
	@rm -f *.h264
	@rm -f *.jpg
	@rm -f *.mjp
	@rm -f *.mp4
	
show:
	@echo $(RTSP_SRC)
	@echo $(RTSP_OBJ)
