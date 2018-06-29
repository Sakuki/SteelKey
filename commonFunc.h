#ifndef COMMONFUNC_H_
#define COMMONFUNC_H_


#ifdef __cplusplus
extern "C" {
#endif

#define INFINITE 0xFFFFFFFF

	
//int flag=0;

typedef struct _FILE_BUFFER
{
	char fileName[100];
	ULONG buffLength;
	BYTE *buff;
}FILE_BUFFER;

typedef struct SteelKey
{
	char steelLine[10];
	char steelAD[10];
	char steelOff[10];
	char steelBus[20];
	char S_short[20];
	char S_long[20];
	char downAtt[10];
	char shortAtt[10];
	char longAtt[10];
	char continueAtt[10];
	char upAtt[10];
	char longStart[10];
	char longSpace[0];
}SteelKey_C;
	
typedef void (*_t_procINI)(BOOL bNode,char *id,char *value,int *flag);



void Sleep(UINT32 sTime);
ULONG GetTickCount(void);

UINT32 stringToDec(char *buf,int len);

void ResetUserTimer(ULONG *Timer);
ULONG ReadUserTimer(ULONG *Timer);
BOOL TimerHasExpired(ULONG *Timer);
void TimerSet(ULONG *Timer, ULONG time);
BOOL TimerOut(ULONG *timer, ULONG timeout);

void fileN( char *fileName);
void readINIFileConfig(char *file, _t_procINI _p_procINI);
void clearINIFileConfig(const char *file);
void readINIFile(const char *fileName,_t_procINI _p_procINI);

void PostSignal(pthread_mutex_t *pMutex,pthread_cond_t *pCond,BOOL *bRunAgain);
int WaitSignedTimeOut(pthread_mutex_t *pMutex,pthread_cond_t *pCond,BOOL *bRunAgain,UINT32 iTimeOutMs);


#ifdef __cplusplus
}
#endif

#endif
