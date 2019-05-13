#ifndef ROBOT
#define ROBOT

#define WINDOWNS_ENV	0	//是否是window下的环境
#define VM_LINUX	1	//linux虚拟机环境
#define OBSERVE_CAMER		//是否观察图片
#define IMAGE_ACCELERATE	//是否使用图像加速
#define MAIN

#include "suitable_exposure.h"
#include "V4l2_setparm.h"
#include <math.h>
#include <stdio.h>
#include<vector>
#include <omp.h>
#include "iostream"

#include "opencv2/opencv.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <opencv2/opencv.hpp>
#include <sys/time.h>
#include <unistd.h>
#include "serials.h"
#include "led.h"
#include "config.h"


typedef struct{
	int xvalue;
	int yvalue;
	int distance2;		//�����ƽ��
	float radius;
	int colour;
	float angle;
}ball_str;

#define BALL_NUM_MAX	10
typedef struct{
	ball_str bal[BALL_NUM_MAX];
	unsigned char num;
}tennis;

void gui_Trackbar_thresh(void);




#endif
