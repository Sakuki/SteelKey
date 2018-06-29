#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <stdio.h>
#include <poll.h>
#include <pthread.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <sys/poll.h> 
#include <sys/stat.h>
#include <sys/times.h>
#include <sys/time.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

#include "types_def.h"

#include "commonFunc.h"




//---------------------------------------

#define INI_FILE_MAX	20
FILE_BUFFER fileConfig[INI_FILE_MAX];
//(filename,length,buf)

UINT32 stringToDec(char *buf,int len);

void procINIData(BYTE *p,UINT length,_t_procINI _p_procINI);


void fileN(char *fileName)
{
	FILE *fd;
	ULONG fsize;
	char *p;
	
	fd = fopen(fileName,"rb");
	if(fd == NULL)
	{
		printf("open %c false",*fileName);
	}
	fseek(fd,0,SEEK_END);
	fsize = ftell(fd);
	fseek(fd,0,SEEK_SET);	
	p = (char *)malloc(fsize);
	fread(p,1,fsize,fd);
	
	strcpy(fileConfig[0].fileName,fileName);
	fileConfig[0].buffLength=fsize;
	fileConfig[0].buff=p;
	printf("fileName is :%s\n",fileConfig[0].fileName);
	printf("buffLength is : %ld\n",fileConfig[0].buffLength);
//	printf("buff is : %s\n",fileConfig[0].buff);
}

void Sleep(UINT32 sTime)
{
	usleep(sTime*1000);	//sTime毫秒
}

ULONG GetTickCount(void)
{
/*
	struct timespec usr_timer;
	clock_gettime(CLOCK_MONOTONIC,&usr_timer);

	CLOCK_MONOTONIC:	从系统启动这一刻起开始计时,不受系统时间被用户改变的影响

	return usr_timer.tv_sec*1000 + usr_timer.tv_nsec / 1000000;
*/

    struct timeval nowTime; 
    gettimeofday(&nowTime, NULL); //获取当前时间
    return nowTime.tv_sec*1000 + nowTime.tv_usec/1000;
		//				
}

void ResetUserTimer(ULONG *Timer)	//复位后当前时间
{
    *Timer = GetTickCount();
}


ULONG ReadUserTimer(ULONG *Timer)	//用户使用的时间？
{
    return GetTickCount() - *Timer;
}

BOOL TimerHasExpired(ULONG *Timer)	//已经失效时间？
{
    if (GetTickCount() - *Timer >= 1)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

void TimerSet(ULONG *Timer, ULONG time)	//设置时间？
{
    *Timer = GetTickCount() + time;
}

BOOL TimerOut(ULONG *timer, ULONG timeout)
{
	if (GetTickCount() - *timer >= timeout)
	{
		*timer = GetTickCount();
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}


void PostSignal(pthread_mutex_t *pMutex,pthread_cond_t *pCond,BOOL *pbRunAgain)
{
	pthread_mutex_lock(pMutex);	//加锁
	*pbRunAgain = TRUE;
	pthread_cond_signal(pCond);	//发信号
	pthread_mutex_unlock(pMutex);	//解锁
}

int WaitSignedTimeOut(pthread_mutex_t *pMutex,pthread_cond_t *pCond,BOOL *pbRunAgain,UINT32 iTimeOutMs)
{
	 int ret = 0;
	struct timeval timenow;
	struct timespec timeout; 
	UINT32 iSecnod,iMSecnod;
	
	pthread_mutex_lock(pMutex);
	if (*pbRunAgain) {   }
	else if (INFINITE == iTimeOutMs || 0 == iTimeOutMs) //INFINITE:无限
	{ 
		pthread_cond_wait(pCond,pMutex); 		//条件等待
	}
	else
	{
		gettimeofday(&timenow, NULL);			//获取当前时间 从1970年1月1日开始
		
		iSecnod = iTimeOutMs / 1000; 			//秒
		iMSecnod = iTimeOutMs % 1000;			//微秒
		
		timeout.tv_sec = timenow.tv_sec + iSecnod; 		//等待时间秒数
		timeout.tv_nsec = (timenow.tv_usec + iMSecnod*1000)*1000;		//等待时间纳秒数
		
		while (timeout.tv_nsec >= 1000000000)
		{
			timeout.tv_nsec -= 1000000000; 
				//1秒=1000000000纳秒
			timeout.tv_sec++;	////秒数增加
		}
		
		ret = pthread_cond_timedwait(pCond,pMutex,&timeout); 
		//条件变量（触发条件），互斥锁，等待时间（系统时间+等待时间）
	}	
	*pbRunAgain = FALSE;
		
	pthread_mutex_unlock(pMutex);	//解锁
	return ret;
	
}


//--------------------- below is flyaudio config file function interface -----------------
//			在飞歌功能设置界面下

UINT32 stringToDec(char *buf,int len)	//将字符型转换成十六进制
{
	UINT32 iData = 0;
	BOOL b16x = FALSE;

	//DBG3(debugPrintf(" stringToDec %s %d\n",buf,len););

	if ('0' == buf[0])
	{
		if ('x' == buf[1] || 'X' == buf[1])	//buf[]={0,x/X}
		{
			b16x = TRUE;
			buf += 2;
			len -= 2;
		}
	}

	while (*buf && len)
	{
		if (b16x)
		{
			if (buf[0] >= 'a' && buf[0] <= 'f')
			{
				iData *= 16;
				iData += buf[0]+10-'a';
			}
			else if (buf[0] >= 'A' && buf[0] <= 'F')
			{
				iData *= 16;
				iData += buf[0]+10-'A';
			}
			else if (buf[0] >= '0' && buf[0] <= '9')
			{
				iData *= 16;
				iData += buf[0]-'0';
			}
		}
		else
		{
			if (buf[0] >= '0' && buf[0] <= '9')
			{
				iData *= 10;
				iData += buf[0]-'0';
			}
		}
		buf++;
		len--;
	}

	return iData;
}
				//uchar				 (bool,id,value)
void procINIData(BYTE *p,UINT length,_t_procINI _p_procINI)		//获取ID和value
{
	UINT i;

	char *id;
	char *value;
	int *flag;

	int idLength = 0;
	int valueLength = 0;
	BOOL isNode = FALSE;

	BOOL bID = TRUE;
	BOOL bNeedProc = FALSE;
	BOOL bEnd = FALSE;

	id = (char *)malloc(256);
	value = (char *)malloc(256);

	for (i = 0;i <= length;i++)
	{
		if (i == length)
		{
			bNeedProc = TRUE;
			bEnd = TRUE;
		}
		else if ('[' == p[i])
		{
			isNode = TRUE;
		}
		else if (']' == p[i])
		{
			bNeedProc = TRUE;
			bEnd = TRUE;
		}
		else if ('#' == p[i])
		{
			bID = TRUE;
			bNeedProc = TRUE;
			bEnd = TRUE;
		}
		else if ('\r' == p[i] || '\n' == p[i])
		{
			bID = TRUE;
			bNeedProc = TRUE;
			bEnd = FALSE;
		}
		else if ('=' == p[i])
		{
			bID = FALSE;
		}						//		空格				～	
		else if (p[i] > ' ')// if (p[i] > 0x20 && p[i] <= 0x7E)
		{
			if (bID)
			{
				id[idLength++] = p[i];
				id[idLength] = '\0';
			}
			else
			{
				value[valueLength++] = p[i];
				value[valueLength] = '\0';
			}
		}

		if (bNeedProc)
		{
			bNeedProc = FALSE;

			if (isNode)		//[ ]里面的内容
			{
				if (idLength)
				{
					flag=(int *)1;
					(*_p_procINI)(TRUE,id,value,flag);
				}
			}
			else
			{
				if (idLength && valueLength)	//键和值
				{
					flag=(int *)0;
					(*_p_procINI)(FALSE,id,value,flag);
				}
			}

			isNode = FALSE;
			idLength = 0;
			valueLength = 0;
		}
	}
	free(id);
	free(value);
}


void readINIFile(const char *fileName,_t_procINI _p_procINI)	//读取二进制文件
{
	FILE *fd;
	ULONG fsize;
	char *p;

	fd = fopen(fileName,"rb");	//二进制读取模式打开文件
	if(fd)
	{
		fseek(fd,0,SEEK_END);	//指向文件结尾
		fsize = ftell(fd);		//用于得到文件位置指针当前位置相对于文件首的偏移字节数
		p = (char *)malloc(fsize);//分配内存，大小与所给予文件一致

		fseek(fd,0,SEEK_SET);	//指向文件头
		fread(p,1,fsize,fd);	//读取文件内容

		procINIData((BYTE *)p,fsize,_p_procINI);
		//			文件内容，文件长度

		free(p);
		fclose(fd);
	}
	else
	{
		//debugPrintf(" flyreadINIFile open -> %s fail!\n",fileName);
		printf(" flyreadINIFile open -> %s fail!\n",fileName);
	}
}

//uchar
BYTE* getINIFileBuff(const char *fileName, ULONG *length)
{
	FILE *fd;
	ULONG fsize;
	BYTE *p;

	fd = fopen(fileName,"rb");
	if(fd)
	{
		fseek(fd,0,SEEK_END);
		fsize = ftell(fd);
		p = (BYTE *)malloc(fsize);

		if (p != NULL)
		{
			fseek(fd,0,SEEK_SET);
			fread(p,1,fsize,fd);
			*length = fsize;
			return p;
		}
	}

	//debugPrintf(" getINIFileBuff %s fail! -> %s\n", fileName, strerror(errno));
	printf(" getINIFileBuff %s fail! -> %s\n", fileName, strerror(errno));
	*length = 0;
	return NULL;

}

void readINIFileConfig(char *file, _t_procINI _p_procINI)
{
	int i = 0;
					//20
	for (i = 0; i < INI_FILE_MAX; i++)
	{
		if (fileConfig[i].buffLength > 0)
		{
			if (strcmp(fileConfig[i].fileName, file) == 0)	//文件名相同时
			{
				procINIData(fileConfig[i].buff, fileConfig[i].buffLength, _p_procINI);
				return ;
			}
		}
		else
		{
			fileConfig[i].buff = getINIFileBuff(file, &(fileConfig[i].buffLength));	//将file内容赋给
			if (fileConfig[i].buff != NULL)
			{
				snprintf(fileConfig[i].fileName, 100, "%s", file);		//将文件的99个字符复制到文件名上
				procINIData(fileConfig[i].buff, fileConfig[i].buffLength, _p_procINI);
			}
			return ;
		}
	}

	if (fileConfig[INI_FILE_MAX-1].buffLength > 0)
	{
		free(fileConfig[INI_FILE_MAX-1].buff);	//释放fileConfig[19]内容的储存空间
		memset(&fileConfig[INI_FILE_MAX-1], 0, sizeof(FILE_BUFFER));	//fileConfig[19]置零	
		fileConfig[INI_FILE_MAX-1].buff = getINIFileBuff(file, &(fileConfig[INI_FILE_MAX-1].buffLength));	//将file内容赋给
		if (fileConfig[INI_FILE_MAX-1].buff != NULL)
		{
			snprintf(fileConfig[INI_FILE_MAX-1].fileName, 100, "%s", file);
			procINIData(fileConfig[INI_FILE_MAX-1].buff, fileConfig[INI_FILE_MAX-1].buffLength, _p_procINI);
		}
	}
}

void clearINIFileConfig(const char *file)		//清空
{
	int i = 0;
	for (i = 0; i < INI_FILE_MAX; i++)
	{
		if (fileConfig[i].buffLength > 0)
		{
			if (strcmp(fileConfig[i].fileName, file) == 0)
			{
				free(fileConfig[i].buff);
				memset(&fileConfig[i], 0, sizeof(FILE_BUFFER));
				return ;
			}
		}
	}
}

