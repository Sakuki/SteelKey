#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>

#include "types_def.h"

char SteelKey[50];
char steelAD[50];
char steelLine[10];
char AD_flag[3];			//是否获取AD值 或者回到上一级
char SteelKey_flag[50];		//输入的方向盘信息是否有误 或者是否退出程序
char steelLine_flag[50];	//输入的line值是否有误 获取返回上一级

int Line_on_flag = 1;		//是否获取Line值
int AD_on_flag = 1;			//是否获取AD值

int fd1;
int fd2;
char buf_read[80];

void pipe_open()	//创建打开管道
{
	unlink("./key_user_fifo");
    int ret1 = mkfifo("./key_user_fifo",0777);
	if(ret1<0)
	{
		perror("mkfifo key_user_fifo error\n");
	}
	
	fd1 = open("./key_user_fifo",O_WRONLY);
	fd2 = open("./../key_key_fifo",O_RDONLY);
	if( (fd1 <0) || (fd2<0) )
	{
		perror("open key_user_fifo failed\n");
	}
}

void pipe_write(char *buf_write)
{
	int ret1 = write(fd1,buf_write,sizeof(buf_write));	//将内容写入到key_user_fifo中
	if(ret1 <= 0)
	{
		printf("write error\n");
	}
}

BYTE* pipe_read()
{
	int ret2 = read(fd2,buf_read,sizeof(buf_read));	//从key_key_fifo中读取内容
	if(ret2 <= 0)
	{
		printf("read end or error\n");
	}
	return buf_read;
}

int main()
{
	pipe_open();
	while(1)
	{
		printf("\n which SteelKey you want? (input \"back\" to leave)\n");
		scanf("%s",SteelKey);
		pipe_write(SteelKey);		//传输所选方向盘的信息
		usleep(10*1000);
		strcpy(SteelKey_flag,pipe_read());
		if(strcmp(SteelKey_flag,"close")==0)	//关闭程序
			break;
		else if(strcmp(SteelKey_flag,"SteelKey")==0)	//输入方向盘信息有误
			continue;
		
		Line_on_flag = 1;
		while(Line_on_flag)
		{
			printf("\n please input a steelLine (input \"back\" to select SteelKey ) \n");
			scanf("%s",steelLine);
			pipe_write(steelLine);
			
			strcpy(steelLine_flag,pipe_read());
			if(strcmp(steelLine_flag,"BTSK")==0)		//返回方向盘选择
				break;
			else if(strcmp(steelLine_flag,"Line")==0)	//Line值有误
				continue;
			AD_on_flag = 1;
			while(AD_on_flag)
			{
				printf("\n please input a steelAD (input \"a\" to select SteelLine \"b\" to select SteelLine ) \n");
				scanf("%s",steelAD);
				pipe_write(steelAD);	//传输所选AD值的信息
				
				strcpy(AD_flag,pipe_read());
				if(strcmp(AD_flag,"back")==0)		//返回Line选择
				{
					AD_on_flag = 0;
					break;
					
				}
				else if(strcmp(AD_flag,"BTSK")==0)	//返回方向盘选择
				{
					AD_on_flag = 0;
					Line_on_flag = 0;
					break;
				}
				else if(strcmp(AD_flag,"AD")==0)	//输入AD值有误
					continue;
			}
			
		}
	}
	close(fd1);
	close(fd2);
	
	return 0;
}