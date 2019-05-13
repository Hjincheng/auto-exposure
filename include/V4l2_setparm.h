#ifndef V4L2_SETPARM
#define V4L2_SETPARM

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <error.h>
#include <string.h>
#include <fcntl.h>
#include <linux/videodev2.h>
#include <sys/ioctl.h>
#include "iostream"


int init_V4l(int VideoNum);
int qctrl_init_parm();
void v4l_set_exposure_absolute(unsigned int arc);
int v4l_get_exposure_absolute();
void v4l_set_brightness(int arc);
int v4l_get_brightness();
void v4l_set_exposure_white_balance(unsigned int arc);
int v4l_get_exposure_white_balance();






typedef struct {
	int id;
	char name[20];
	int min;
	int max;
	int suit_value;
}camer_parm_str;

#endif
