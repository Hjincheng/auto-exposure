#include "suitable_exposure.h"
#include "robot.h"

#define E_NUM	13
//曝光的设置值
int const exposure_SValue[13] = {1, 2, 5, 10,
								 20,39,78,156,
								 312,625,1250,2500,
								 5000};

extern VideoCapture capture;

int brightness;//亮度
int contrast;//对比度
int saturation;//饱和度
int tone;//色调
int exposure;//曝光
int gain;		//增益

//#define camer_debug		//打开配置调试窗口调试摄像头
//#define		MAIN
#define CAMER_DUBUG		//打印摄像头配置参数
#define P_EXPOSURE	(0.25)		//曝光的PID 的P值
#define EXPOSURE_LEN	(12 - 3)
#define EXPOSURE_MINNUM	1


#define EXPOSURE_MIN	(0)
#define EXPOSURE_MAX	(2500)
#define BRIGHTNESS_MIN	(-30)
#define BRIGHTNESS_MAX	(30)
#define BEAR_VALUE		(20)

int energy_exp = 85;			//期望能量值
int white_balance_temperature = 0;	//白平衡





void gui_Trackbar_energy_exp(void);



void parameter_brightness(int, void*)
{
	capture.set(CV_CAP_PROP_BRIGHTNESS, brightness);//亮度
	printf("亮度 = %.2f\n", capture.get(CV_CAP_PROP_BRIGHTNESS));//亮度
}

void parameter_contrast(int, void*)
{
	capture.set(CV_CAP_PROP_CONTRAST, contrast);//对比度 40
	printf("对比度 = %.2f\n", capture.get(CV_CAP_PROP_CONTRAST));//对比度
}

void parameter_saturation(int, void*)
{
	capture.set(CV_CAP_PROP_SATURATION, saturation);//饱和度 50
	printf("饱和度 = %.2f\n", capture.get(CV_CAP_PROP_SATURATION));//饱和度
}

void parameter_tone(int, void*)
{
	capture.set(CV_CAP_PROP_HUE, tone);//色调 50
	printf("色调 = %.2f\n", capture.get(CV_CAP_PROP_HUE));//色调
}

void parameter_exposure(int, void*)
{
	capture.set(CV_CAP_PROP_EXPOSURE, -(13 - exposure));//曝光 50
	printf("曝光 = %.2f\n", capture.get(CV_CAP_PROP_EXPOSURE));//曝光
}

void parameter_gain(int, void*)
{
	capture.set(CV_CAP_PROP_GAIN, gain);//增益
	printf("增益 = %.2f\n", capture.get(CV_CAP_PROP_GAIN));//增益
}

/*!
*  @function	int compare_exposure(int value)
*  @author 	黄锦成
*  @e-mail	895816513@163.com
*  @brief	获取当前的曝光等级（返回第一个不小于曝光数组的对应的号数，否则返回最大的号数）
*  @param	value	输入的曝光值
*  @time	2018-4-6
*  return num		返回曝光数组exposure_SValue[num]的对应的号数
*  Sample usage:
*				int energy = Get_image_energy(RGB_Image);
*/
int compare_exposure(int value)
{
	int num = 0;

	while(exposure_SValue[num] < value)
	{
		if((num+1)<E_NUM)
			num++;
		else
			break;
	}
	return num;
}

void my(Mat src_RGB_img)
{
	
	//int i;
	int i = 0;
	int j = 0;
	Mat YUV_img;
	cvtColor(src_RGB_img, YUV_img, CV_RGB2YUV);	//转换为YUV图像
	double Y_sum = 0;
	int width = YUV_img.rows;			//图像宽度
	int height = YUV_img.cols;			//图像高度
	//int DibWidth = YUV_img.step;		//图像行大小
	int depth = YUV_img.depth();
	//int channels = YUV_img.channels();

	if (0)
		for (; i < width; i++)
		{
			for (; j < height; j++)
			{
				Y_sum = YUV_img.at<Vec3f>(i, j)[0];
			}
			//imshow("images", YUV_img);
		}
		
	cout << "The width of Y is " << width << endl;
	cout << "The height of Y is " << height << endl;
	cout << "The depth of Y is " << depth << endl;
	cout <<"The sum of Y is"<< Y_sum << endl;
	

};


int fun1(Mat Image)
{
	Mat img_h, img_s, img_v, imghsv;
	
	//Image = imread("D://Libs//opencv310//opencv//sources//samples//data//aero1.jpg");      //原图
	vector<cv::Mat> hsv_vec;
	cvtColor(Image, imghsv, CV_BGR2HSV);

	// 分割hsv通道  
	split(imghsv, hsv_vec);
	img_h = hsv_vec[0];
	img_s = hsv_vec[1];
	img_v = hsv_vec[2];
	img_h.convertTo(img_h, CV_32F);
	img_s.convertTo(img_s, CV_32F);
	img_v.convertTo(img_v, CV_32F);
	double max_s, max_h, max_v;
	minMaxIdx(img_h, 0, &max_h);
	minMaxIdx(img_s, 0, &max_s);
	minMaxIdx(img_v, 0, &max_v);

	//输入RGB图像,转换成YUV并分离
	Mat imageY(Image.rows, Image.cols, 1);
	Mat imageU(Image.rows, Image.cols, 1);
	Mat imageV(Image.rows, Image.cols, 1);

	Mat imageYUV;
	cvtColor(Image, imageYUV, CV_BGR2YUV);
	vector<Mat> mv;
	split(Image, (vector<Mat>&)mv);

	imageY = mv[0].clone();
	imageU = mv[1].clone();
	imageV = mv[2].clone();

	//bgr
	for (int i = 0; i < 3; i++)
	{
		Mat bgr(Image.rows, Image.cols, CV_8UC3, Scalar(0, 0, 0));
		Mat temp(Image.rows, Image.cols, CV_8UC1);
		Mat out[] = { bgr };
		int from_to[] = { i, i };
		mixChannels(&Image, 1, out, 1, from_to, 1);
		//分别显示bgr
		//imshow("bgr", bgr);
		//waitKey();
	}

	//显示
	//imshow("Y", imageY);
	int height = imageY.rows;
	int width = imageY.cols;
	long Y_num = 0;
	//int Y_num = src.at<Vec3b>(row, col)[0]; // blue
	Mat gray_src;

	for (int row = 0; row < height; row++) //行循环
	{
		for (int col = 0; col < width; col++) //列循环
		{
			
			Y_num += imageY.at<uchar>(row, col);//灰度图像下，通道只有一个
		}
	}
	cout << "The brightness is " << Y_num << endl;
	cout << "The per brightness is " << Y_num / height / width << endl;
	//imshow("output", gray_src);

	return 0;
}

/*!
*  @function	Get_image_energy(Mat const &RGB_Image)
*  @author 	黄锦成
*  @e-mail	895816513@163.com
*  @brief	获取图像能量（亮度）
*  @param	RGB_Image	输入的RGB图像
*  @time	2018-3-6
*  return average_brightness		输入RGB图像的亮度
*  Sample usage:
*				int energy = Get_image_energy(RGB_Image);
*/
int Get_image_energy(Mat const &RGB_Image)
{
	//输入RGB图像,转换成YUV并分离
	Mat imageY(RGB_Image.rows, RGB_Image.cols, 1);
	//Mat imageU(Image.rows, Image.cols, 1);
	//Mat imageV(Image.rows, Image.cols, 1);

	Mat imageYUV;
	cvtColor(RGB_Image, imageYUV, CV_BGR2YUV);
	vector<Mat> mv;
	split(imageYUV, (vector<Mat>&)mv);		//转换为矩阵操作

	imageY = mv[0].clone();		//矩阵的Y
	//imageU = mv[1].clone();		//矩阵的U
	//imageV = mv[2].clone();		//矩阵的V

	//显示
	//imshow("Y", imageY);
	int height = imageY.rows;	//图像的行数
	int width = imageY.cols;	//图像的列数
	long sum_brightness = 0;

	for (int row = 0; row < height; row++) //行循环
	{
		for (int col = 0; col < width; col++) //列循环
		{

			sum_brightness += imageY.at<uchar>(row, col);//灰度图像下，通道只有一个
		}
	}

	//图像亮度就是灰度图像的平均值，就是Y的平均值
	int average_brightness = sum_brightness / height / width;		//平均每个像素的亮度

	//cout << "The average brightness is " << average_brightness << endl;

	return average_brightness;
}



/*!
*  @function	void my_set_exposure_absolute(int vale)
*  @author 	黄锦成
*  @e-mail	895816513@163.com
*  @brief	有保护的设置摄像头曝光，避免输入参数过大或过小
*  @param	value	输入的曝光值
*  @time	2018-4-6
*  return num		返回曝光数组exposure_SValue[num]的对应的号数
*  Sample usage:
*				int energy = Get_image_energy(RGB_Image);
*/

void my_set_exposure_absolute(int vale)
{
	if(vale > EXPOSURE_MAX)
	{
		vale = EXPOSURE_MAX;
	}
	else if(vale < EXPOSURE_MIN)
	{
		vale = EXPOSURE_MIN;
	}
	v4l_set_exposure_absolute(vale);
	cout << "after suit_expourse = ： " << vale << endl;
}


int my_get_exposure_absolute(void)
{
	return v4l_get_exposure_absolute();
}

void my_set_brightness(int exposure)
{
	v4l_set_brightness(exposure);
}

int my_get_brightness(void)
{
	return v4l_get_brightness();
}


void gain_rgb_img(Mat &dst,Mat &src,double gain)
{
	//Mat dst;
	//dst.create(src.size(), src.type());
	int height = dst.rows;
	int width = dst.cols;
	//int channels = dst.channels();
	for (int row = 0; row < height; row++) //行循环
	{
		for (int col = 0; col < width; col++) //列循环
		{
			int b = src.at<Vec3b>(row, col)[0]; // blue
			int g = src.at<Vec3b>(row, col)[1]; // green
			int r = src.at<Vec3b>(row, col)[2]; // red

			//四舍五入
			int b_cal = (int)((b)*gain + 0.5);
			int g_cal = (int)((g)*gain + 0.5);
			int r_cal = (int)((r)*gain + 0.5);

			dst.at<Vec3b>(row, col)[0] = b_cal > 255 ? 255:b_cal;//(((b)*gain + 0.5)> 255) ? 255:((b)*gain + 0.5)> 255;
			dst.at<Vec3b>(row, col)[1] = g_cal > 255 ? 255:g_cal; 
			dst.at<Vec3b>(row, col)[2] = r_cal > 255 ? 255:r_cal;

		}
	}
	//imshow("output", dst);
}

void wait(long delay)
{
	return;
	for (delay = 100 * delay; delay > 0; delay--)
	for (int jjj = 30; jjj > 0; jjj--)
		;// cout << delay << endl;
}


/*!
*  @function	void get_suitable_img(Mat &dst, Mat  & scr)
*  @author 	黄锦成
*  @e-mail	895816513@163.com
*  @brief	通过该图像获取曝光合适的图像，一般在过几帧图像后才有效果，
*			如果没有定义IMAGE_ACCELERATE，则会强行使用数字调节图像能量亮度，可能会是噪声增加。
*  @param	dst	输出图像，一般不使用输出图像，如果没有定义IMAGE_ACCELERATE，则才有效果
*  @param	scr	输入图像,
*  @time	2018-4-6
			2019年4月3日 修正PID逻辑
*  Sample usage:
*				get_suitable_img(dst,scr);
*/

void get_suitable_img(Mat &dst, Mat  & scr)
{	
	static int brightcnt = 0;		//极限曝光计数器
	wait(2);

	
	

	int cur_expourse = my_get_exposure_absolute();		//获取当前摄像头曝光值
	int cur_brightness = my_get_brightness();			//获取当前摄像头亮度值
	static int Nextposure = compare_exposure(cur_expourse);		//计算当前的曝光等级

	if(1)//while (waitKey(1))
	{
		//imshow("images", scr);		
		int energy = Get_image_energy(scr);	//获得图像能量
		float k_of_brightness = (float)energy_exp / energy;  //期望亮度与实际亮度的比
		float PID_err	= (energy_exp - energy);		//期望图像能量与实际图像能量的差值
		signed int value ;//= (signed int)(P_EXPOSURE * PID_err);		//将偏差乘以一个放大系数

		if(PID_err > 1)
		{
			value = (P_EXPOSURE * PID_err) + 1;
		}
		else if(PID_err < -1)
		{
			value = (P_EXPOSURE * PID_err) - 1;
		}
			
		else
			value = 0;
			

		cout << "期望亮度\t" << energy_exp << endl;
		cout << "实际亮度\t" << energy << endl;

#ifdef CAMER_DUBUG
		cout << "suit当前曝光\t" << cur_expourse << endl;
		cout << "camer当前亮度\t" << cur_brightness << endl;
		
		printf("期望亮度与实际亮度的比 ：%0.2f\n", k_of_brightness);
#endif
		if(abs(PID_err)> 5)		//可容忍的余差
		{
			cur_brightness += value;
			//cout << "suit_expourse \t= " << cur_brightness << endl;
			cout<<"__LINE__"<<endl;
			cout<<"abs(PID_err) = "<<abs(PID_err)<<endl;

			if(cur_brightness >= BRIGHTNESS_MAX)
			{
				cur_brightness = BRIGHTNESS_MAX;

				if(abs(PID_err)> BEAR_VALUE)	//可容忍的余差
					brightcnt++;
				else
					return;
				
				if(brightcnt > 10)
				{
					brightcnt = 0;		//计算清零
					Nextposure++;
					if(Nextposure > EXPOSURE_LEN)
						Nextposure = EXPOSURE_LEN;

					my_set_exposure_absolute(exposure_SValue[Nextposure]);
					//调节摄像头brightness到中间值
					my_set_brightness((BRIGHTNESS_MIN + BRIGHTNESS_MAX)/2);
				}
			}
			else if(cur_brightness <= BRIGHTNESS_MIN)
			{
				cur_brightness = BRIGHTNESS_MIN;

				if(abs(PID_err)> BEAR_VALUE)	//可容忍的余差
					brightcnt--;
				else
					return;
				
				if(brightcnt < -10)
				{
					brightcnt = 0;		//计算清零
					Nextposure--;
					
					if(Nextposure < EXPOSURE_MINNUM)
						Nextposure = EXPOSURE_MINNUM;

					my_set_exposure_absolute(exposure_SValue[Nextposure]);
					my_set_brightness((BRIGHTNESS_MIN + BRIGHTNESS_MAX)/2);
						
				}
			}
			else
				my_set_brightness(cur_brightness);
			//wait(50);		//等待一段时间，因为读寄存器需要时间，没反应过来
			//cout << "after suit_expourse = ： " << suit_expourse << endl;
		}

		cout << "Nextposure \t= " << Nextposure << endl;
		cout << "my_get_exposure_absolute() \t= " << my_get_exposure_absolute() << endl;
		
		return;
		
#ifdef CAMER_DUBUG

		
		cout << "cur_expourse \t= " << cur_expourse << endl;
		cout << "PID_err = "<< PID_err<< endl;
		cout << "brightcnt \t= " << brightcnt << endl;
		cout << "cur_brightness \t= " << cur_brightness << endl;
#endif


#ifndef IMAGE_ACCELERATE
		gain_rgb_img(dst, scr, k_of_brightness);		//直接修改图像亮度，以达到目标亮度
		int persent_energy = Get_image_energy(dst);
		
#ifdef CAMER_DUBUG
		cout << "修改后图像的实际亮度 ： " << persent_energy << endl;
#endif
#endif

	}
	
}


void gui_Trackbar_camer(void)
{	
	//namedWindow("CameraParameter");srcimage
	namedWindow("srcimage");
	createTrackbar("对比度", "srcimage", &contrast, 64, parameter_contrast);//0 - 64
	createTrackbar("饱和度", "srcimage", &saturation, 128, parameter_saturation);//0 - 128
	createTrackbar("色调", "srcimage", &tone, 40, parameter_tone);//-40 - 40
	//createTrackbar("曝光", "CameraParameter", &exposure, 13, parameter_exposure);//-13 - -1
	createTrackbar("增益", "srcimage", &gain, 100, parameter_gain);//0 - 100
	
	parameter_contrast(0, 0);
	//while(1);
}

void notuse(int, void*)
{
	return;
}


void gui_Trackbar_energy_exp(void)
{	
	//namedWindow("CameraParameter");srcimage
	namedWindow("energy_exp");
	createTrackbar("energy_exp", "energy_exp", &energy_exp, 130, notuse);//0 - 64
	
	notuse(0, 0);
	//while(1);
}



#ifndef		MAIN

int main()
{
	//capture.open(0);
	//Mat scr;
	//Mat suitable_RGB_img;
	//capture >> scr;
	//suitable_RGB_img.create(scr.size(), scr.type());

	int ret = 0;
	int ymax = 0, xmax = 0;

	int noFindCount = 0;
	Mat element = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(5, 5));

	capture.open(0);
	Mat srcimage;
	Mat suitable_RGB_img;
	capture >> srcimage;
	suitable_RGB_img.create(srcimage.size(), srcimage.type());

#ifdef camer_debug
	brightness = (int)capture.get(CV_CAP_PROP_BRIGHTNESS);
	contrast = (int)capture.get(CV_CAP_PROP_CONTRAST);
	saturation = (int)capture.get(CV_CAP_PROP_SATURATION);
	tone = (int)capture.get(CV_CAP_PROP_HUE);
	exposure = 13 + (int)capture.get(CV_CAP_PROP_EXPOSURE);
	gain = (int)capture.get(CV_CAP_PROP_GAIN);

	namedWindow("CameraParameter");
	//createTrackbar("亮度", "CameraParameter", &brightness, 64, parameter_brightness);//-64 - 64
	createTrackbar("对比度", "CameraParameter", &contrast, 64, parameter_contrast);//0 - 64
	createTrackbar("饱和度", "CameraParameter", &saturation, 128, parameter_saturation);//0 - 128
	createTrackbar("色调", "CameraParameter", &tone, 40, parameter_tone);//-40 - 40
	//createTrackbar("曝光", "CameraParameter", &exposure, 13, parameter_exposure);//-13 - -1
	createTrackbar("增益", "CameraParameter", &gain, 100, parameter_gain);//0 - 100

#endif


	while(waitKey(1))
	{
		capture >> srcimage;//scr;
		imshow("srcimage", srcimage);//scr);
		get_suitable_img(suitable_RGB_img, srcimage);
	}

	return 0;
}

#endif


