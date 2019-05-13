/**************************************************************************************
 * @文件名：	serials.cpp
 * @功能描述： 串口初始化和串口发送函数
 * ----------------------------------------------------------------------
 * 修改日期        版本号     修改人	      修改内容
 * 2016/12/10  	V1.0		薛文旺	      创建
 ***************************************************************************************/


 #include "serials.h"

/*****************************************************************************************
* @方法名:    serials     
* @描述:     构造方法
* @权限:     public
* @形参:     name --设备节点名
* @返回值:    无
* ---------------------------------------------------------------------
* @修改日期	@版本号    @修改人	   @修改内容
* 2016/12/10    V1.0	  	   薛文旺	    创建
*****************************************************************************************/
serials::serials(char * name)
{
	this->name = name;
}



/*****************************************************************************************
* @函数名：set_opt
* @描述：    设置串口波特率、数据位数、有无校验位和停止位数
* @权限：    public
* @形参：    nSpeed--波特率，支持2400、4800、9600、115200、921600，默认9600
		     nBits--数据位数  
		     nEvent--有无校验位，'O'表示奇校验，'E'表示偶校验，'N'表示无校验
		     nStop--停止位数，1和2位
* @返回值：0--成功		-1--失败		 
*-------------------------------------------------------------------------------------------------------
*@修改日期	@版本号	@修改人	@修改内容
1016/12/10	V1.0		薛文旺	创建
*****************************************************************************************/
int serials::set_opt(int nSpeed, int nBits, char nEvent, int nStop)
{
	struct termios newtio,oldtio;
	if  ( tcgetattr( fd,&oldtio)  !=  0) 
	{ 
		perror("SetupSerial 1");
		return -1;
	}
	bzero( &newtio, sizeof( newtio ) );
	newtio.c_cflag  |=  CLOCAL | CREAD; 
	newtio.c_cflag &= ~CSIZE; 

	switch( nBits )
	{
		case 7:
			newtio.c_cflag |= CS7;
			break;
		case 8:
			newtio.c_cflag |= CS8;
			break;
	}

	switch( nEvent )
	{
		case 'O':                     //奇校验
			newtio.c_cflag |= PARENB;
			newtio.c_cflag |= PARODD;
			newtio.c_iflag |= (INPCK | ISTRIP);
			break;
		case 'E':                     //偶校验
			newtio.c_iflag |= (INPCK | ISTRIP);
			newtio.c_cflag |= PARENB;
			newtio.c_cflag &= ~PARODD;
			break;
		case 'N':                    //无校验
			newtio.c_cflag &= ~PARENB;
			break;
	}

	switch( nSpeed )
	{
		case 2400:
			cfsetispeed(&newtio, B2400);
			cfsetospeed(&newtio, B2400);
			break;
		case 4800:
			cfsetispeed(&newtio, B4800);
			cfsetospeed(&newtio, B4800);
			break;
		case 9600:
			cfsetispeed(&newtio, B9600);
			cfsetospeed(&newtio, B9600);
			break;
		case 115200:
			cfsetispeed(&newtio, B115200);
			cfsetospeed(&newtio, B115200);
			break;
		case 921600:
			cfsetispeed(&newtio, B921600);
			cfsetospeed(&newtio, B921600);
			break;
		default:
			cfsetispeed(&newtio, B9600);
			cfsetospeed(&newtio, B9600);
			break;
	}

	if( nStop == 1 )
	{
		newtio.c_cflag &=  ~CSTOPB;
	}
	else if ( nStop == 2 )
	{
		newtio.c_cflag |=  CSTOPB;
	}

	newtio.c_cc[VTIME]  = 0;
	newtio.c_cc[VMIN] = 0;
	tcflush(fd,TCIFLUSH);
	if((tcsetattr(fd,TCSANOW,&newtio))!=0)
	{
		perror("com set error");
		return -1;
	}
	
	printf("set done!\n");
	return 0;
}


/**********************************************************************************
* @函数名：open_port
* @描述：    打开串口
* @权限：    public
* @形参：    无
* @返回值：返回文件句柄， -1--打开失败  
*----------------------------------------------------------------------------------------------
* @修改日期	@版本号	@修改人	@修改内容
* 2016/12/10    v1.0		薛文旺	创建
*********************************************************************************/
int serials::open_port()
{
	fd = open(name, O_RDWR|O_NOCTTY|O_NDELAY);
	if (-1 == fd)
	{
		perror("Can't Open Serial Port");
		return(-1);
	}
	else 
	{
		printf("open %s .....\n",name);
	}	
	
	if(fcntl(fd, F_SETFL, 0)<0)
	{
		printf("fcntl failed!\n");
	}
	else
	{
		printf("fcntl=%d\n",fcntl(fd, F_SETFL,0));
	}
	
	if(isatty(STDIN_FILENO)==0)
	{
		printf("standard input is not a terminal device\n");
	}
	else
	{
		printf("isatty success!\n");
	}
	
	printf("open %s success\n", name);
	
	return fd;
}



/**********************************************************************************
* @函数名：send
* @描述：    串口发送数据
* @权限：    public
* @形参：    buf--数据源地址， num--数据长度
* @返回值：无 
*----------------------------------------------------------------------------------------------
* @修改日期	@版本号	@修改人	@修改内容
* 2016/12/10    v1.0		薛文旺	创建
*********************************************************************************/	
void serials::send(char *buf, int num)
{
	if(write(fd, buf, num) < 0)
		printf("serials send error\n");
       else
	  {
		printf("send is ok\n");
	  } 
}
