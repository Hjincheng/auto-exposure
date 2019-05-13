#include "led.h"

/**********************************************************************************************************
* @文件名：	led.cpp
* @功能描述：  用户空间访问gpio，即通过sysfs接口访问gpio，下面是/sys/class/gpio目录下的三种文件： 
			--export/unexport文件
			--gpioN指代具体的gpio引脚
			--gpio_chipN指代gpio控制器
*		            
* ----------------------------------------------------------------------
* 修改日期        版本号     修改人	      修改内容
* 2016/12/15  	V1.0		薛文旺	      创建
***********************************************************************************************************/




/*********************************************************************************
* @函数名：gpio_export
* @描述：    导出gpio端口到用户空间
* @权限：    public
* @形参：    pin--引脚端口
* @返回值：0--成功		1--失败
*----------------------------------------------------------------------------------------
* @修改日期	@版本号	@修改人	@修改内容
* 2016/12/15    v1.0		薛文旺	创建
*********************************************************************************/	
int gpio_export(int pin)  
{  
	char buffer[64];  
	int len;  
	int fd;  

	fd = open("/sys/class/gpio/export", O_WRONLY);  
	if (fd < 0) 
	{  
		printf("Failed to open export for writing!\n");  
		return(-1);  
	}  

	len = snprintf(buffer, sizeof(buffer), "%d", pin);  
	if (write(fd, buffer, len) < 0) 
	{  
		printf("Failed to export gpio!\n");  
		return -1;  
	}  

	close(fd);  
	return 0;  
}  

/*********************************************************************************
* @函数名：gpio_unexport
* @描述：    撤销导出到用户空间的gpio端口
* @权限：    public
* @形参：    pin--引脚端口
* @返回值：0--成功		1--失败
*----------------------------------------------------------------------------------------------
* @修改日期	@版本号	@修改人	@修改内容
* 2016/12/15    v1.0		薛文旺	创建
*********************************************************************************/	
int gpio_unexport(int pin)  
{  

	char buffer[64];  
	int len;  
	int fd;  
	fd = open("/sys/class/gpio/unexport", O_WRONLY);  

	if (fd < 0)
	{  
		printf("Failed to open unexport for writing!\n");  
		return -1;  
	}  

	len = snprintf(buffer, sizeof(buffer), "%d", pin);  
	if (write(fd, buffer, len) < 0) 
	{  
		printf("Failed to unexport gpio!\n");  
		return -1;  
	}  

	close(fd);  
	return 0;  
} 


 
/*********************************************************************************
* @函数名：gpio_direction
* @描述：    设置gpio的使用方向是输入还是输出 
* @权限：    public
* @形参：    pin--引脚端口；	 dir: 0-->IN,  1-->OUT
* @返回值：0--成功		 1--失败
*----------------------------------------------------------------------------------------------
* @修改日期 @版本号  @修改人  @修改内容
* 2016/12/15	 v1.0		 薛文旺	 创建
*********************************************************************************/  
int gpio_direction(int pin, int dir)  
{  
	static const char dir_str[] = "in\0out";  
	char path[64];  
	int fd;  

	snprintf(path, sizeof(path), "/sys/class/gpio/gpio%d/direction", pin);  
	fd = open(path, O_WRONLY);  
	if (fd < 0) 
	{  
		printf("Failed to open gpio direction for writing!\n");  
		return -1;  
	}  

	if (write(fd, &dir_str[dir == 0 ? 0 : 3], dir == 0 ? 2 : 3) < 0) 
	{  
		printf("Failed to set direction!\n");  
		return -1;  
	}  

	close(fd);  
	return 0;  
}  


/*********************************************************************************
* @函数名：gpio_write
* @描述：    设置gpio引脚的值
* @权限：    public
* @形参：    pin--引脚端口；	 value: 0-->LOW, 1-->HIGH
* @返回值：0--成功		 1--失败
*----------------------------------------------------------------------------------------------
* @修改日期 @版本号  @修改人  @修改内容
* 2016/12/15	 v1.0		 薛文旺	 创建
*********************************************************************************/  
int gpio_write(int pin, int value)  
{  

	static const char values_str[] = "01";  
	char path[64];  
	int fd;  

	snprintf(path, sizeof(path), "/sys/class/gpio/gpio%d/value", pin);  
	fd = open(path, O_WRONLY);  
	if (fd < 0) 
	{  
		printf("Failed to open gpio value for writing!\n");  
		return -1;  
	}  

	if (write(fd, &values_str[value == 0 ? 0 : 1], 1) < 0) 
	{  
		printf("Failed to write value!\n");  
		return -1;  
	}  

	close(fd);  
	return 0;  
}


/*********************************************************************************
* @函数名：ledInit
* @描述：    led初始化函数
* @权限：    public
* @形参：    pin--引脚端口；	dir: 0-->IN,  1-->OUT；	 value: 0-->LOW, 1-->HIGH
* @返回值：0--成功	
*----------------------------------------------------------------------------------------------
* @修改日期 @版本号  @修改人  @修改内容
* 2016/12/15	 v1.0		 薛文旺	 创建
*********************************************************************************/  
int ledInit(int pin, int dir, int value)
{
	gpio_export(pin);
	gpio_direction(pin, dir);
	gpio_write(pin, value);
	
	return 0;
}
