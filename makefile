CROSS_COMPILE 	= 
AS		= $(CROSS_COMPILE)as
LD		= $(CROSS_COMPILE)ld
CC		= $(CROSS_COMPILE)gcc
#CPP		= $(CC) -E
CPP		= $(CROSS_COMPILE)g++
AR		= $(CROSS_COMPILE)ar
NM		= $(CROSS_COMPILE)nm
STRIP		= $(CROSS_COMPILE)strip
OBJCOPY		= $(CROSS_COMPILE)objcopy
OBJDUMP		= $(CROSS_COMPILE)objdump


export AS LD CC CPP AR NM
export STRIP OBJCOPY OBJDUMP

CFLAGS := -g -Wall -O2
CFLAGS += -I $(shell pwd)/include

CPPFLAGS := -g -Wall -O2 -std=c++11
CPPFLAGS += -I $(shell pwd)/include

LDFLAGS := -lopencv_core -lopencv_highgui -lopencv_imgproc -lopencv_ml -lpthread
INCLUDE	:= $(shell pkg-config opencv --cflags)
LIBS	:= $(shell pkg-config opencv --libs)
LIBS	+= -lopencv_core -lopencv_highgui -lopencv_imgproc -lopencv_ml -lpthread


export CFLAGS LDFLAGS

TOPDIR := $(shell pwd)
export TOPDIR

TARGET := robot

obj-y += serials.o
obj-y += led.o
obj-y += robot.o
obj-y += suitable_exposure.o
obj-y += V4l2_setparm.o

all :
	make -C ./ -f $(TOPDIR)/makefile.build
	$(CPP) $(INCLUDE) -o $(TARGET) built-in.o -L $(LIBS)
#	$(CPP) $(INCLUDE) -o $(TARGET) built-in.o $(LIBS)
#	$(CPP) $(LDFLAGS) -o $(TARGET) built-in.o
	
run: $(TARGET)	
#	sudo rm /dev/video0
#	sudo ln -s /dev/video9 /dev/video0
#	DISPLAY=:0 ./$(TARGET)
	sudo ./$(TARGET)
	
clean:
	rm -f $(shell find -name "*.o")
	rm -f $(TARGET)
	
distclean:
	rm -f $(shell find -name "*.o")
	rm -f $(shell find -name "*.d")
	rm -f $(TARGET)

