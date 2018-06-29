#include <stdio.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>

#include "types_def.h"
#include "commonFunc.h"

#define Steel_Key_MAX	10
SteelKey_C SteelKey_config[Steel_Key_MAX];
char steelOff[Steel_Key_MAX];

char SteelKey[50];
char steelAD[50];
char steelLine[10];
char steelOff[10];
int f=0;	//SteelKey标志位
int nSK=1;	//SteelKey不存在标志位
int ADf=0;	//AD标志位
int nAD=1;	//AD不存在标志位
int nLine=1;//Line不存在标志位
int nend=1;	//结束程序标志位
int AD_proc_flag=1;	//AD值处理
int steelLine_flag=1;//SteelLine值处理
int sub_flag=1,add_flag=1;
int num,numValue,numOff,numOther,numOff_Other,add_steelOff,sub_steelOff;
float new_numOff;
int i,j,k;		//第几个按键

pthread_mutex_t AD_mutex;
pthread_cond_t user_cond;
BOOL bl= 0;

int fd1;	//key_user_fifo
int fd2;
char buf_read[80];

void pipe_open()		//创建打开管道
{
	unlink("./key_key_fifo");
    int ret2 = mkfifo("./key_key_fifo",0777);
	if(ret2<0)
	{
		perror("mkfifo key_key_fifo error\n");
	}
	fd1 = open("./user/key_user_fifo",O_RDONLY);
	fd2 = open("./key_key_fifo",O_WRONLY);
	if( (fd1 <0) || (fd2<0) )
	{
		perror("open file failed\n");
	}
	
}

BYTE* pipe_read()
{
	int ret1 = read(fd1,buf_read,sizeof(buf_read));	//从key_user_fifo中读取内容
	if(ret1 <= 0)
	{
		printf("read end or error\n");
	}
	return buf_read;
}

void pipe_write(char *buf_write)
{
	int ret2 = write(fd2,buf_write,sizeof(buf_write));	//将内容写入到key_key_fifo中
	if(ret2 <= 0)
	{
		printf("write end or error\n");
	}
}



void find_key_config(BOOL bNode,char *id,char *value,int *flag)	//解析.ini文件  打印出用户输入的方向盘的按键的配置
{
	
	if(bNode==FALSE)	//键和值
	{
		if(strcmp(id,"name")==0)
		{
			if(strcmp(value,SteelKey)==0)
			{
				printf("The SteelKey you select is : %s\n",SteelKey);
				pipe_write("find");
				f=1;
				nSK=0;
				AD_proc_flag=1;
				steelLine_flag=1;
			}
		}
	}
	if(f && flag)
	{
		printf("\n\n[%s]\n",id);
		if( strcmp(id,"SteelID_1")==0 )		//获取当前在第几个按键
			i=1;
		if( strcmp(id,"SteelID_2")==0 )
			i=2;
		if( strcmp(id,"SteelID_3")==0 )
			i=3;
		if( strcmp(id,"SteelID_4")==0 )
			i=4;
		if( strcmp(id,"SteelID_5")==0 )
			i=5;
		if( strcmp(id,"SteelID_6")==0 )
			i=6;
		if( strcmp(id,"SteelID_7")==0 )
			i=7;
		if( strcmp(id,"SteelID_8")==0 )
			i=8;
		if( strcmp(id,"SteelID_9")==0 )
			i=9;
		if( strcmp(id,"SteelID_10")==0 )
			i=10;
		
	}
	if(f && flag==0)
	{
		printf("%s=%s\n",id,value);			//将按键配置赋值到方向盘配置结构体中
		if( strcmp(id,"steelLine")==0 )
			strcpy(SteelKey_config[i].steelLine,value);
		else if( strcmp(id,"steelAD")==0 )
			strcpy(SteelKey_config[i].steelAD,value);
		else if( strcmp(id,"steelOff")==0 )
			strcpy(SteelKey_config[i].steelOff,value);
		else if( strcmp(id,"steelBus")==0 )
			strcpy(SteelKey_config[i].steelBus,value);
		else if( strcmp(id,"short")==0 )
			strcpy(SteelKey_config[i].S_short,value);
		else if( strcmp(id,"long")==0 )
			strcpy(SteelKey_config[i].S_long,value);
		else if( strcmp(id,"downAtt")==0 )
			strcpy(SteelKey_config[i].downAtt,value);
		else if( strcmp(id,"shortAtt")==0 )
			strcpy(SteelKey_config[i].shortAtt,value);
		else if( strcmp(id,"longAtt")==0 )
			strcpy(SteelKey_config[i].longAtt,value);
		else if( strcmp(id,"continueAtt")==0 )
			strcpy(SteelKey_config[i].continueAtt,value);
		else if( strcmp(id,"upAtt")==0 )
			strcpy(SteelKey_config[i].upAtt,value);
		else if( strcmp(id,"longStart")==0 )
			strcpy(SteelKey_config[i].longStart,value);
		else if( strcmp(id,"longSpace")==0 )
			strcpy(SteelKey_config[i].longSpace,value);
	}
	if(bNode==TRUE)		//[]的内容
	{
		if(strcmp(id,"End")==0)
		{
			f=0;
		}
	}
}

void find_key(char *S_short)				//打印出用户选择的方向盘、AD值和其AD值代表的按键
{
	printf("\nThe SteelKey you select is : %s\n",SteelKey);
	
	printf("\nThe SteelLine you select is :%s\n",steelLine);
	
	printf("The steelAD you select is : %d\n",num);
	
	printf("The key you push is %s\n\n",S_short);
	
	pipe_write("find");		//找到AD
}

void *AD_proc(void *arg)
{
	while(1)
	{
		WaitSignedTimeOut(&AD_mutex,&user_cond,&bl,0);
		num = stringToDec(steelAD,4);
		for(j=1;j<11;j++)
		{
			if( strcmp(SteelKey_config[j].steelLine , steelLine)==0)
			{
				sub_steelOff = stringToDec(SteelKey_config[j].steelOff,4);		//左范围值，取设定值
				add_steelOff= stringToDec(SteelKey_config[j].steelOff,4);		//右范围值，取设定值
						
				for(k=1;k<11;k++)
				{
					if(strcmp(SteelKey_config[k].steelLine , steelLine)==0)
					{
						numValue = stringToDec(SteelKey_config[j].steelAD,4);		//获取第j个按键的AD值（被比较）
						numOther = stringToDec(SteelKey_config[k].steelAD,4);		//获取第k个按键的AD值（做比较）
						numOff = stringToDec(SteelKey_config[j].steelOff,4);		//获取第j个按键的AD范围值
						numOff_Other = stringToDec(SteelKey_config[k].steelOff,4);	//获取第k个按键的AD范围值
						
						if( (abs)(numValue-numOther) <= numOff+numOff_Other )		//计算范围值是否重合
						{
							new_numOff = (fabs)(numValue-numOther)/2+0.5;
							if(numValue>numOther)
							{
								sub_steelOff = new_numOff-1;				//左边范围值
								sub_flag = 0;
							}
							else if(numValue<numOther)
							{
								add_steelOff = new_numOff;				//右边范围值
								add_flag = 0;
							}
						}
					}
				}
				if( numValue-sub_steelOff <= num && num <= numValue+add_steelOff )			//将每一个按键的AD值与用户输入的AD值做比较
				{
					find_key(SteelKey_config[j].S_short);
					nAD = 0;
				}
			}
			
		}
		if(nAD)
		{
			if(num==1019)
			{
				printf("All keys not push\n\n");
			}
			else
			{
				printf("This AD is false\n\n");
			}
			pipe_write("AD");
			continue;
		}
		nAD=1;
	}
}

void *user(void *arg)		//子线程 用以获取用户的命令 判断
{
	while(1)
	{
		strcpy(SteelKey,pipe_read());
		if(strcmp(SteelKey,"back")==0)		//关闭程序
		{
			nend=0;
			pipe_write("close");
			break;
		}
		readINIFile("./SteelKeyV2.ini",find_key_config);	//解析.ini文件
		if(nSK)
		{
			printf("This SteelKey is false\n\n");
			pipe_write("SteelKey");
			continue;
		}
		steelLine_flag = 1;
		while(steelLine_flag)
		{
			strcpy(steelLine,pipe_read());
			if(strcmp(steelLine,"back")==0)		//返回上一级
			{
				steelLine_flag = 0;
				pipe_write("BTSK");
				continue;
			}
			for(j=1;j<11;j++)
			{
				if(strcmp(steelLine,SteelKey_config[j].steelLine)==0)
				{
					nLine = 0;
					break;
				}
			}
			if(nLine)		//Line值不存在
			{
				pipe_write("Line");
				printf("Thie SteelLine is fail\n\n");
				continue;
			}
			AD_proc_flag = 1;
			nLine = 1;
			pipe_write("find");		//找到line
			while(AD_proc_flag)
			{
				strcpy(steelAD,pipe_read());
				if(strcmp(steelAD,"a")==0)	//返回上一级
				{
					AD_proc_flag = 0;
					pipe_write("back");
					continue;
				}
				else if(strcmp(steelAD,"b")==0)	//返回方向盘
				{
					steelLine_flag = 0;
					AD_proc_flag = 0;
					pipe_write("BTSK");
					continue;
				}
				PostSignal(&AD_mutex,&user_cond,&bl);
			}
		}
		
		
		nSK=1;
	}
}

void main()
{
	pipe_open();
	pthread_mutex_init(&AD_mutex,NULL);
	pthread_cond_init(&user_cond,NULL);
	
	pthread_t tid,tid2;
	
	int etid = pthread_create(&tid,NULL,user,NULL);
	if(etid<0)
	{
		printf("user pthread_create fail\n");
	}
	pthread_create(&tid2,NULL,AD_proc,NULL);
	if(etid<0)
	{
		printf("AD_proc pthread_create fail\n");
	}
	while(nend)	;
	pthread_mutex_destroy(&AD_mutex);
	pthread_cond_destroy(&user_cond);
}