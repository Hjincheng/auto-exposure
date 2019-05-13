/*****************************************************************************************
* 文件名：    robot.cpp
* 功能描述： 使用opencv提供的函数读取摄像头数据，识别出网球，把识别出来的角度
*		   和距离通过串口发送给单片机，去掉显示
*
* ------------------------------------------------------------
* 修改日期		版本号		修改人		修改内容
* 2016/12/15	V1.0		薛文旺		创建
*							招展鹏		创建
* 2018/3/5		V2.0		黄锦成		修改(增加摄像自动曝光算法)
******************************************************************************************/

#include "robot.h"

using namespace std;
using namespace cv;
VideoCapture capture;

int VideoNumber = 0;
ball_str DistMinBall;//距离最近的球
char buf[CMD_SIZE];  //存放要发送的数据
tennis robot_tennis = {0};
int close_oper = 2;		//闭操作
int threshold_value = 128;	//二值化阈值
Mat close_oper_kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(5, 5));;

int switch_algor_exposure = 1;//自动曝光算法开关




#if (VM_LINUX)
#else
serials ser(SERIAL_NAME); //定义串口对象
#endif
int com = 0;

#define Proportion                                   0.0954//比例，实际长度（cm）：图像分辨率，25:262

//int init(); //初始化


#define VIDEO_WIDTH		(320)
#define VIDEO_HEIGHT	(240)



#if (VM_LINUX)
int init()
{
	init_V4l(VideoNumber);

	//namedWindow("srcimage");
	gui_Trackbar_thresh();
	capture.open(VideoNumber);	//打开/dev/video0

	if (!capture.isOpened())
	{
		cout << "can not open the camera" << endl;  //打不开摄像头			
		return -1;
	}

	/* 设置分辨率 */
	capture.set(CV_CAP_PROP_FRAME_WIDTH, VIDEO_WIDTH);
	capture.set(CV_CAP_PROP_FRAME_HEIGHT, VIDEO_HEIGHT);

	/* 设置线程数 */
	cvSetNumThreads(5);

	return 0;

	
}
#else
int init()
{
	init_V4l(VideoNumber);
	//gui_Trackbar_camer();
	gui_Trackbar_thresh();
	memset(buf, 0, CMD_SIZE);

	/* 协议头部信息 */
	buf[0] = 0x84;
	buf[1] = 0x64;
	buf[2] = 0x44;

	/* 打开串口 */
	if (ser.open_port() < 0){
		/* 如果失败，则打印出：can not open the /dev/ttySACXX */
		cout << "can not open the SERIAL_NAME" << endl;
		return -1;
	}
         cout << "open the ok" << endl;

	/* 设置串口波特率、数据位数、有无校验和停止位数 */
	if (ser.set_opt(BUAD, 8, 'N', 1) < 0){
		cout << "can not set_opt SERIAL_NAME" << endl;
		return -1;
	}

	com = ser.fd;  //com 保存串口文件句柄，以后可通过com读写数据

	capture.open(VideoNumber);	//打开/dev/video0

	if (!capture.isOpened())
	{
		cout << "can not open the camera" << endl;  //打不开摄像头			
		return -1;
	}

	/* 设置分辨率 */
	capture.set(CV_CAP_PROP_FRAME_WIDTH, VIDEO_WIDTH);
	capture.set(CV_CAP_PROP_FRAME_HEIGHT, VIDEO_HEIGHT);

	/* 设置线程数 */
	cvSetNumThreads(5);

	/* LED引脚初始化，引脚方向为输出，初始值为低电平 */
	ledInit(LED_PIN, 1, 0);
	ledInit(LED_WORK, 1, 0);
	
	return 0;
}
#endif

/*****************************************************************************************
* @function：	Get_Binary_Image
* @功能描述：获取二值化图像和HSV色图像，其中包括腐蚀、膨胀等操作
* @param：	dst_Binary_Image	目标二值化图像
* @param:	hsvimage			目标的HSV色图像
* @param:	SrcRGB_Image		输入的RGB图像
* @param:	element				元素
* @author：
*	薛文旺		创建
*	招展鹏		创建
*	黄锦成		整合、注释、优化
* @version：
*	2016/12/15	V1.0		薛文旺 招展鹏
*	2018/3/5	V2.0		黄锦成	895816513@163.com
******************************************************************************************/
void Get_Binary_Image(Mat &dst_Binary_Image, Mat &hsvimage, Mat const &SrcRGB_Image, Mat &element)
{
	cvtColor(SrcRGB_Image, hsvimage, COLOR_BGR2HSV); //把RGB颜色空间转换为HSV颜色空间
	/* 室外参数 */
	//inRange(hsvimage, Scalar(0, 20, 160), Scalar(35, 150, 255), hsvimage);
	/* 室内参数 */
	inRange(hsvimage, Scalar(25, 25, 130), Scalar(70, 240, 255), hsvimage);
	//erode(hsvimage, hsvimage, element); //腐蚀操作		
	//dilate(hsvimage, hsvimage, element); //膨胀操作

#ifdef OBSERVE_CAMER
	imshow("hsvimage111",hsvimage);
#endif
	morphologyEx(hsvimage, dst_Binary_Image, CV_MOP_CLOSE, element);//闭操作
	//dst_Binary_Image = hsvimage;
	//threshold(hsvimage, dst_Binary_Image, threshold_value, 255, THRESH_BINARY); // 二值化 CV_THRESH_OTSU ,当图像数值大于130时，该像素点设为255
}


/*****************************************************************************************
* @function：	conver_position
* @功能描述：将图像中的网球位置信息进行变化，计算得到网球实际的平面位置信息
* @param：	ball	网球的信息
* @image	ball	源输入图像，用于提取图像宽/高度

*	2018/5/15	V2.0		黄锦成	895816513@163.com
******************************************************************************************/
void conver_position(ball_str        &ball, Mat const &image)
{
	//ball_str dst_ball;
	//float a = abs(ball.xvalue - SetPointX);
	float a = abs(ball.xvalue - image.cols/2);		//x方向基准点为 ，图像宽度的一半
	float b = abs(ball.yvalue - (image.rows + 13 / Proportion));//15
//	float b = abs(ball.yvalue - 0);//15
	float c = sqrt(a*a + b*b);//设定点与网球坐标的距离

	//asin，反正弦函数，输出为弧度，公式转换成角度
	float angle = asin(b / c) * 180.0 / 3.1415926;

	cout<<"angle"<<angle<<endl;
	
	//if (ball.xvalue <= SetPointX)//网球在右边
	if (ball.xvalue <= image.cols/2)//网球在右边
	{
		angle -= 60;
		cout<<"angle - 60= "<<angle<<endl;
	}
	else
	{
		angle = 120 - angle;
		cout<<"120 - angle= "<<angle<<endl;
	}

	float yy = c * Proportion;	//计算值-相对于实际环境
	yy = yy - 2;		
	yy = yy * 10;			//单片机需求
	angle = angle * 10;		//单片机需求

	buf[4] = (int)yy / 10;
	buf[5] = (int)yy % 10; //网球与机器人的距离
	buf[6] = (int)angle / 10;
	buf[7] = (int)angle % 10; //网球与机器人的角度
}


/*****************************************************************************************
* @function：void find_ball(Mat const &Binary_Image,Mat &srcimage)
* @功能描述：通过二值化图像找圆
* @param：	Binary_Image		输入的的二值化图像
* @param:	SrcRGB_Image		输入的源RGB图像，供调试观察使用
* @author：
*	薛文旺		创建
*	招展鹏		创建
*	黄锦成		整合、注释、优化
* @version：
*	2016/12/15	V1.0		薛文旺 招展鹏
*	2018/5/15	V2.0		黄锦成	895816513@163.com
******************************************************************************************/
void find_ball(Mat const &Binary_Image,Mat &SrcRGB_Image)
{
	vector<vector<Point> > contours1;
	vector<Vec4i> hierarchy1;
	Point2f center;//网球中心坐标封装
	float radius;//网球轮廓半径
	robot_tennis.num = 0;		//初始化
	int temp_distance = 0;		//临时距离变量

	//从二值图像中找出对象的轮廓
	findContours(Binary_Image, contours1, hierarchy1, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));

	//ymax = 0;//距离最近的网球的位置
	for (unsigned int ii = 0; ii < contours1.size(); ii++)
	{
		double tmparea_1 = 0;//圆形面积
		double tmparea = fabs(contourArea(contours1[ii]));//求轮廓点集的面积

		minEnclosingCircle(Mat(contours1[ii]), center, radius);//包围的最小面积
		tmparea_1 = 3.14*(radius)*(radius);//圆面积
		cout << "The radius is " << radius << endl;

		//轮廓面积 < 包围最小圆面积 * 0.4（只看到月牙形的图形）
		//包围最小圆面积不在此范围内
		if (center.y<400 && tmparea < tmparea_1*0.6) {
			continue;  //误判
		}


//		if (center.y < 200){
//			continue;
//		}

		if (radius < 24){
			continue;
		}

		if (radius > 120){

			continue;
		}

		
		/* 保存距离robot最近的点 */
		if (center.y >= 0){
			//ymax = center.y;
			//xmax = center.x;
			if(robot_tennis.num < BALL_NUM_MAX)
			{	//利用robot_tennis保存符合规则的网球结构，此后应用大小排序和路径规划等，因为黄锦成懒没写，而且也没人需要
				robot_tennis.bal[robot_tennis.num].xvalue = center.x;
				robot_tennis.bal[robot_tennis.num].yvalue = center.y;
				robot_tennis.bal[robot_tennis.num].distance2 = center.x*center.x + center.y*center.y;
				robot_tennis.num++;
			}


			if(temp_distance < (center.x*center.x + center.y*center.y))
			{
				temp_distance = (center.x*center.x + center.y*center.y);

				DistMinBall.xvalue = center.x;
				DistMinBall.yvalue = center.y;
				DistMinBall.radius = radius;
				//cout << " DistMinBall.xvalue\t" << DistMinBall.xvalue << endl;
				//cout << " DistMinBall.yvalue\t" << DistMinBall.yvalue << endl;
				//cout << " DistMinBall.radius\t" << DistMinBall.radius << endl;
			}

#ifdef	OBSERVE_CAMER
								cout << "draw circle which radius is " << radius << endl;
								Mat RGB_notation_img = SrcRGB_Image;
								circle(RGB_notation_img, center, 3, Scalar(0, 255, 0), -1, 8, 0);//绘制球心
								circle(RGB_notation_img, center, radius, Scalar(0, 0, 255), 2, 8, 0);//绘制外轮廓	
								Point2f StdPoint = center;//坐标基准点
								cout << " SrcRGB_Image.cols" << SrcRGB_Image.cols << endl;
								cout << " SrcRGB_Image.rows" << SrcRGB_Image.rows << endl;
								StdPoint.x = SrcRGB_Image.cols / 2;
								//
								//StdPoint.y = SrcRGB_Image.rows / 6;
								circle(RGB_notation_img, StdPoint, radius/2, Scalar(255, 255, 255), 2, 8, 0);//绘制外轮廓
						
								imshow("网球识别", RGB_notation_img); 
#endif

		}
		

		
				//#ifdef OBSERVE_CAMER



	}


}


/*****************************************************************************************
* @function：	send_ball_message()
* @功能描述：发生网球坐标等信息
* @param：	dst_Binary_Image	目标二值化图像
* @author：
*	薛文旺		创建
*	招展鹏		创建
*	黄锦成		整合、注释、优化
* @version：
*	2016/12/15	V1.0		薛文旺 招展鹏
*	2018/5/15	V2.0		黄锦成	895816513@163.com
******************************************************************************************/

#define FIND_BALL	1
#define NFIND_BALL	0
void send_ball_message(int state)
{
	cout<<"send_ball_message"<<endl;
	//通过点灯在调试过程中发现有没有发现球
	if(state == NFIND_BALL)
	{
		buf[3] = 0x22; //协议头，表示没有识别到网球
		gpio_write(LED_PIN, 0); //灭灯
	}	
	else
	{
		buf[3] = 0x21; 			//协议头，表示识别到网球
		gpio_write(LED_PIN, 1); //点灯
	}
		
	//通过串口向单片机发生信息
	tcflush(com, TCOFLUSH); //刷新串口输出缓冲区
	if(write(com, buf, 8) < 0) //把位置信息发送给单片机
	{
		cout<<"write error ! In the '"<< __FILE__<<"' "<< __LINE__ <<" line."<< endl;
	}

	
}

void feedback_f(int, void *)
{
	//腐蚀膨胀
	close_oper_kernel = getStructuringElement(MORPH_RECT, Size(close_oper*2 + 1,close_oper*2 + 1));

	if(white_balance_temperature < 2800){
		white_balance_temperature = 2800;
	}
	v4l_set_exposure_white_balance(white_balance_temperature);
}


char DEBUG_WIN[] = "二值化图像";
char dst_win[] = "自动曝光";



void gui_Trackbar_thresh(void)
{
	
#ifndef OBSERVE_CAMER
	return;
#endif
	namedWindow(dst_win);
	//createTrackbar("CV_MOP_CLOSE", dst_win, &close_oper, 255, feedback_f);
	//createTrackbar("threshold_value", dst_win, &threshold_value, 255, feedback_f);
	createTrackbar("energy_exp", dst_win, &energy_exp, 130, feedback_f);//0 - 64
	createTrackbar("switch_algor_exposure", dst_win, &switch_algor_exposure, 1, feedback_f);//0 - 64
	
	white_balance_temperature = v4l_get_exposure_white_balance();
	//white_balance_temperature 范围 2800 -6500
	createTrackbar("white_balance_temperature", dst_win, \
					&white_balance_temperature, 6500, feedback_f);	
	feedback_f(0, 0);
}


//输入二值化图像，计算白点（255）的平均坐标
Point get_averge_point(const Mat &thresh_img)
{
	int height = thresh_img.rows;
	int width = thresh_img.cols;
	
	Point averge(0,0);
	long X_count = 0;
	long Y_count = 0;
	long count = 0;
	
	//int Y_num = src.at<Vec3b>(row, col)[0]; // blue
	Mat gray_src;

	for (int row = 0; row < height; row++) //行循环
	{
		for (int col = 0; col < width; col++) //列循环
		{
			if(thresh_img.at<uchar>(row, col) == 255) //灰度图像下，白点
			{
				X_count += col;
				Y_count += row;
				count++;
				
			}
		}
	}

	if(count > 180)
	{
		averge.x = X_count/count;
		averge.y = Y_count/count;
	}
	else
		averge.x = averge.y = 0;

	cout << "白色点 count："<< count << endl;

	return averge;
}




int g_param = 0;
int g_param2 = 6;
int g_param3 = 2;
int g_param4 = 3;


Point averge_point(0,0);


void xiangjian(const Mat &new_rgb_img, const Mat &old_rgb_img)
{
	//灰度
	Mat new_gray_img,old_gray_img;
	cvtColor(new_rgb_img, new_gray_img, CV_BGR2GRAY);	//将背景图片进行灰度变化
	cvtColor(old_rgb_img, old_gray_img, CV_BGR2GRAY);				//将目标图片进行灰度变化


	//blur 平均滤波
	/**/
	Mat new_gray_img_blur, old_gray_img_blur, dogImg;
	blur(new_gray_img, new_gray_img_blur, Size(g_param*2 + 1,g_param*2 + 1));
	blur(old_gray_img, old_gray_img_blur, Size(g_param*2 + 1 ,g_param*2 + 1));

	//subtract(old_gray_img_blur, new_gray_img_blur, dogImg, Mat());
	//subtract(new_gray_img, old_gray_img, dogImg, Mat());
	absdiff(new_gray_img_blur, old_gray_img_blur, dogImg);

	//imshow("new_gray_img_blur", new_gray_img_blur);
	//imshow("old_gray_img_blur", old_gray_img_blur);
	
	
	//dogImg = old_gray_img_blur - new_gray_img_blur;

	//归一化显示
	normalize(dogImg, dogImg, 255, 0, NORM_MINMAX);

	imshow("dogImg",dogImg);

	//腐蚀膨胀
	Mat kernel = getStructuringElement(MORPH_RECT, Size(g_param3*2 + 1,g_param3*2 + 1));
	//morphologyEx(dogImg, dogImg, MORPH_OPEN, kernel);//闭操作

	morphologyEx(dogImg, dogImg, MORPH_ERODE, kernel);	//腐蚀

	kernel = getStructuringElement(MORPH_RECT, Size(g_param4*2 + 1,g_param4*2 + 1));
	morphologyEx(dogImg, dogImg, MORPH_DILATE, kernel);//膨胀

	imshow("腐蚀膨胀",dogImg);
	//二值化图像
	Mat thresh_dogImg;
	int thresh = g_param2;
	threshold(dogImg, thresh_dogImg, thresh, 255, THRESH_BINARY);

	//获取图像中心点
	averge_point =get_averge_point(thresh_dogImg);
	cout<< "averge_point.x = "<<averge_point.x;
	cout<< " averge_point.y = "<<averge_point.y<<endl;

	imshow(DEBUG_WIN,thresh_dogImg);

	//dst_img = thresh_dogImg;
	//imshow("闭操作",dst_img);
}



Rect my_get_Rect( InputOutputArray _img, Point pt, int len)
{
	Point pt1, pt2;

	pt1.x = ((pt.x - len) < 0)? 0: (pt.x - len);
	pt1.y = ((pt.y - len) < 0)? 0: (pt.y - len);

	cout<<"pt1.x = "<< pt1.x<<"pt1.y = "<<pt1.y<<endl;

	pt2.x = ((pt.x + len) >= VIDEO_WIDTH)? (VIDEO_WIDTH - 1): (pt.x + len);
	pt2.y = ((pt.y + len) >= VIDEO_HEIGHT)? (VIDEO_HEIGHT - 1): (pt.y + len);

	cout<<"pt2.x = "<< pt2.x<<"pt2.y = "<<pt2.y<<endl;

	Rect rect(pt1.x, pt1.y, pt2.x - pt1.x, pt2.y - pt1.y);

	return 	rect;
}



int main(int argc, char** argv)
{
	int ret = 0;
	
	Mat srcimage,img_old;		//原图像
	Mat suitable_RGB_img;	//优化后的图像
	Mat hsvimage;			//HSV色图像
	Mat Binary_Image;		//二值化图像

	//创建图像空间
	suitable_RGB_img.create(srcimage.size(), srcimage.type());
	
	ret = init(); //初始化摄像头、串口、LED引脚等等
	waitKey(1500);

	if (ret< 0)
	{
		printf("init error\n!"); //初始化错误
		return -1;
	}

	namedWindow(DEBUG_WIN, WINDOW_AUTOSIZE);
	createTrackbar("模糊", DEBUG_WIN, &g_param, 250, feedback_f);
	createTrackbar("thresh", DEBUG_WIN, &g_param2, 250, feedback_f);
	createTrackbar("腐蚀", DEBUG_WIN, &g_param3, 20, feedback_f);
	createTrackbar("膨胀", DEBUG_WIN, &g_param4, 20, feedback_f);
	feedback_f(0, 0);

	capture >> srcimage; //读取一帧图像
	img_old = srcimage.clone();

#ifdef OBSERVE_CAMER
	while(waitKey(20))
#else
	while (1)
#endif
	{
		capture >> srcimage; //读取一帧图像

		//imshow("srcimage", srcimage);
		//imshow("img_old", img_old);
		
		xiangjian(srcimage, img_old);
		img_old = srcimage.clone();

		if (srcimage.empty())
		{
			printf("没有图像\n");
			continue;
		}		
		
#ifdef	OBSERVE_CAMER
		static Point point_backup(0,0);
		static Rect rect;
		Mat cut_img;

		
		//averge_point在xiangjian()里面更新
		if((averge_point.x != 0) || (averge_point.y != 0))
			point_backup = averge_point;	//备份中心点
			
		rect = my_get_Rect(srcimage, point_backup, 60);//绘制外轮廓
			
		if(rect.width != 0 && rect.height != 0)
		{			
			cut_img = srcimage(rect);
			if(switch_algor_exposure)
				get_suitable_img(suitable_RGB_img, cut_img);
			//imshow("剪接", srcimage(rect));
			//绘制外轮廓
			rectangle(srcimage, rect, Scalar(0, 0, 255), 2, 8, 0);
			//circle(srcimage, point_backup, 50, Scalar(0, 0, 255), 2, 8, 0);
		}

		imshow(dst_win, srcimage);
#endif
	}

	return 0;
}



