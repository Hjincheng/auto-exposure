#include "V4l2_setparm.h"

int fd;
//const char *input_dev = "/dev/video0";
char input_dev[50]={0};
const char *qctrl_name = "Brightness";//NULL;
int qctrl_value = 60;//0;

struct v4l2_capability cap;
struct v4l2_queryctrl qctrl;
using namespace std;


camer_parm_str camer_parm_buff[5];








/*
 = {

	{
		.id 		= V4L2_CID_SATURATION;
		.suit_value = 82;
	},
	{
		.id 		= V4L2_CID_HUE;
		.suit_value	= 17;
	},
	{
		.id 		= V4L2_CID_AUTO_WHITE_BALANCE;
		.suit_value	= 1;
	},	
	{
		.id 		= V4L2_CID_GAMMA;
		.suit_value	= 100;
	},
	{
		.id 		= V4L2_CID_GAIN;
		.suit_value	= 8;
	}

};

*/



static int set_value(int id, int value)
{
	struct v4l2_control ctrl;

	ctrl.id = id;
  	ctrl.value = value;
  	return ioctl(fd, VIDIOC_S_CTRL, &ctrl);
}

static int qctrl_get_parm(int id)
{
  struct v4l2_control ctrl;

  ctrl.id = id;
  if (ioctl(fd, VIDIOC_G_CTRL, &ctrl) < 0) {
    printf("get ctrl failed");
    ctrl.value = -999;
  }

  return ctrl.value;
}

static void qctrl_set_parm(int id, int value)
{
    qctrl.id = id;
	
    if (ioctl(fd, VIDIOC_QUERYCTRL, &qctrl) == 0) {
	   set_value(id, value);
    }
	else
		printf("Set parameter error ID: 0x%x ",id);
		
}

void v4l_set_exposure_absolute(unsigned int arc)
{
	qctrl_set_parm(V4L2_CID_EXPOSURE_ABSOLUTE, arc);
}

int v4l_get_exposure_absolute()
{
	int value = qctrl_get_parm(V4L2_CID_EXPOSURE_ABSOLUTE);
	//printf("v4l_get_exposure %d\n",value);
	return value;
}

void v4l_set_exposure_white_balance(unsigned int arc)
{
	qctrl_set_parm(V4L2_CID_WHITE_BALANCE_TEMPERATURE,arc);;
}

int v4l_get_exposure_white_balance()
{
	int value = qctrl_get_parm(V4L2_CID_WHITE_BALANCE_TEMPERATURE);
	//printf("v4l_get_exposure %d\n",value);
	return value;
}



void v4l_set_brightness(int arc)
{
	qctrl_set_parm(V4L2_CID_BRIGHTNESS,arc);
}

int v4l_get_brightness()
{
	int value = qctrl_get_parm(V4L2_CID_BRIGHTNESS);
	//printf("v4l_get_exposure %d\n",value);
	return value;
}


/*****************************************************************************************
* @function：	qctrl_init_parm()
* @功能描述：初始化摄像头，配置寄存器
* @param：	dst_Binary_Image	目标二值化图像
* @author：
*	黄锦成		创建
* @version：
*	2018/5/15	V2.0		黄锦成	895816513@163.com
******************************************************************************************/
int qctrl_init_parm()
{
	//camer_parm_str *pp = &camer_parm_buff[0];
	//int i = sizeof(camer_parm_buff)/sizeof(camer_parm_buff[0]);
	qctrl_set_parm(V4L2_CID_CONTRAST, 45);
	qctrl_set_parm(V4L2_CID_SATURATION, 82);
	qctrl_set_parm(V4L2_CID_HUE, 17);
	qctrl_set_parm(V4L2_CID_AUTO_WHITE_BALANCE, 0);	//关闭自动白平衡
	qctrl_set_parm(V4L2_CID_GAMMA, 100);
	qctrl_set_parm(V4L2_CID_GAIN, 8);
	qctrl_set_parm(V4L2_CID_EXPOSURE_AUTO, V4L2_EXPOSURE_MANUAL);//设置曝光为手动模式
	cout<<"V4L2_CID_WHITE_BALANCE_TEMPERATURE:"<<qctrl_get_parm(V4L2_CID_WHITE_BALANCE_TEMPERATURE)<<endl;

	return 0;
}


int init_V4l(int VideoNum)
{
	int ret;
	sprintf(input_dev,"/dev/video%d",VideoNum);
	fd = open(input_dev, O_RDWR);
	printf("%s",input_dev);
	
	if (fd < 0) {
		perror("open video failed");
		return -1;
	}
	printf("open video '%s' success\n", input_dev);

	ret = ioctl(fd, VIDIOC_QUERYCAP, &cap);
	if (ret < 0) {
		while(1)
		cout<<"could not get the "<<input_dev<<" ctrl"<<endl;
	}
	if ((cap.capabilities &  V4L2_CAP_VIDEO_CAPTURE) == 0) {
		printf("video device donot support capture\n");
		return -1;
	}
	
	qctrl_init_parm();	//初始化化摄像头
	
	return 0;
}

