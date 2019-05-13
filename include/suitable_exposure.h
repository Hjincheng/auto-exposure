#ifndef SUITABLE_EXPOSURE
#define SUITABLE_EXPOSURE

#include "robot.h"
#include "iostream"
#include "opencv2/highgui/highgui.hpp"  // Video write

#include "opencv2/opencv.hpp"


using namespace std;
using namespace cv;

extern int energy_exp ;			//期望能量值
extern int white_balance_temperature; //白平衡

extern void get_suitable_img(Mat &dst, Mat  & scr);
extern void gui_Trackbar_energy_exp(void);
void gui_Trackbar_camer(void);
#endif