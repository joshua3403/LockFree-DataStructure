#include "stdafx.h"
#include "CCrashDumpClass.h"
#include "MemoryPool(LockFree).h"
#include "Stack(LockFree).h"
#include "Queue(LockFree).h"

#define dfMAX_DATA_COUNT 3

struct st_DATA
{
	LONG64 lData;
	LONG64 lCount;
};


BOOL bShutDown = FALSE;
LONG64 LAllocCount = 0;
LONG64 LPushCount = 0;
LONG64 LPopCount = 0;
CFreeList<st_DATA> g_MemoryPool(0,FALSE);
st_DATA* aDataInitial[dfMAX_DATA_COUNT];
CLockFreeStack<st_DATA*> g_LockFreeStack;
CQueue<st_DATA*> g_LockFreeQueue;
LONG64 lPushTPS = 0;
LONG64 lPopTPS = 0;
SYSTEM_INFO si;


unsigned int WINAPI MemoryPoolThreadFunc(VOID* arg);
unsigned int WINAPI LockFreeStackThreadFunc(VOID* arg);
unsigned int WINAPI LockFreeQueueThreadFunc(VOID* arg);


void PrintMemoryPool(void);
void InitialMemoryPool(void);

void InitialLockFreeStack(void);
void PrintLockFreeStack(void);

void InitialLockFreeQueue(void);
void PrintLockFreeQueue(void);

int main()
{
	CCrashDump::CCrashDump();
	std::vector<HANDLE> threadhandle;

	GetSystemInfo(&si);


	DWORD dwThreadID = 0;
	DWORD t = 0;

	InitialLockFreeQueue();

	for (DWORD i = 0; i < si.dwNumberOfProcessors; i++)
	{
		threadhandle.push_back((HANDLE)_beginthreadex(NULL, 0, LockFreeQueueThreadFunc, NULL, 0, (unsigned int*)&dwThreadID));
		CloseHandle(threadhandle[i]);
	}

	while (true)
	{
		if (GetAsyncKeyState('q') & 0x8000 || GetAsyncKeyState('Q') & 0x8000)
		{
			bShutDown = true;
			break;
		}
		PrintLockFreeQueue();
		Sleep(999);
		
	}

	wprintf(L"Stoped\n");
	return 0;
}

unsigned int __stdcall MemoryPoolThreadFunc(VOID* arg)
{
	st_DATA* aTestData[1000];
	while (!bShutDown)
	{
		for (int i = 0; i < 1000; i++)
		{
			aTestData[i] = g_MemoryPool.Alloc();
			InterlockedIncrement64(&LAllocCount);
		}

		for (int i = 0; i < 1000; i++)
		{
			if ((aTestData[i]->lData != 0x0000000055555555) || (aTestData[i]->lCount
				!= 0))
				CCrashDump::Crash();
		}

		for (int i = 0; i < 1000; i++)
		{
			InterlockedIncrement64(&aTestData[i]->lData);
			InterlockedIncrement64(&aTestData[i]->lCount);
		}

		for (int i = 0; i < 1000; i++)
		{
			if ((aTestData[i]->lData != 0x0000000055555556) || (aTestData[i]->lCount
				!= 1))
				CCrashDump::Crash();
		}

		for (int i = 0; i < 1000; i++)
		{
			InterlockedDecrement64(&aTestData[i]->lData);
			InterlockedDecrement64(&aTestData[i]->lCount);
		}

		for (int i = 0; i < 1000; i++)
		{
			if ((aTestData[i]->lData != 0x0000000055555555) || (aTestData[i]->lCount
				!= 0))
				CCrashDump::Crash();
		}

		for (int i = 0; i < 1000; i++)
		{
			g_MemoryPool.Free(aTestData[i]);
			InterlockedDecrement64(&LAllocCount);
		}
	}


	return 0;
}

unsigned int __stdcall LockFreeStackThreadFunc(VOID* arg)
{
	st_DATA* pDataArray[1000];

	for (int i = 0; i < 1000; i++)
	{
		if (!(g_LockFreeStack.Pop(&pDataArray[i])))
			CCrashDump::Crash();
	}

	for (int i = 0; i < 1000; i++)
	{
		if (pDataArray[i]->lData != 0x0000000055555555 || pDataArray[i]->lCount != 0)
			CCrashDump::Crash();
	}

	for (int i = 0; i < 1000; i++)
	{
		InterlockedIncrement64(&pDataArray[i]->lData);
		InterlockedIncrement64(&pDataArray[i]->lCount);
	}

	for (int i = 0; i < 1000; i++)
	{
		if (pDataArray[i]->lData != 0x0000000055555556 || pDataArray[i]->lCount != 1)
			CCrashDump::Crash();
	}

	for (int i = 0; i < 1000; i++)
	{
		InterlockedDecrement64(&pDataArray[i]->lData);
		InterlockedDecrement64(&pDataArray[i]->lCount);
	}

	for (int i = 0; i < 1000; i++)
	{
		if (pDataArray[i]->lData != 0x0000000055555555 || pDataArray[i]->lCount != 0)
			CCrashDump::Crash();
	}

	for (int i = 0; i < 1000; i++)
	{
		if (!(g_LockFreeStack.Push(pDataArray[i])))
			CCrashDump::Crash();

		InterlockedIncrement64(&LPushCount);

	}

	return 0;
}

unsigned int __stdcall LockFreeQueueThreadFunc(VOID* arg)
{
	st_DATA* pDataArray[dfMAX_DATA_COUNT];

	while (!bShutDown)
	{
		for (int i = 0; i < dfMAX_DATA_COUNT; i++)
		{
			if (!(g_LockFreeQueue.Dequeue(&pDataArray[i])))
				CCrashDump::Crash();

			InterlockedIncrement64(&LPopCount);
		}

		for (int i = 0; i < dfMAX_DATA_COUNT; i++)
		{
			if (pDataArray[i]->lData != 0x0000000055555555 || pDataArray[i]->lCount != 0)
				CCrashDump::Crash();
		}

		for (int i = 0; i < dfMAX_DATA_COUNT; i++)
		{
			InterlockedIncrement64(&pDataArray[i]->lData);
			InterlockedIncrement64(&pDataArray[i]->lCount);
		}

		for (int i = 0; i < dfMAX_DATA_COUNT; i++)
		{
			if (pDataArray[i]->lData != 0x0000000055555556 || pDataArray[i]->lCount != 1)
				CCrashDump::Crash();
		}

		for (int i = 0; i < dfMAX_DATA_COUNT; i++)
		{
			InterlockedDecrement64(&pDataArray[i]->lData);
			InterlockedDecrement64(&pDataArray[i]->lCount);
		}

		for (int i = 0; i < dfMAX_DATA_COUNT; i++)
		{
			if (pDataArray[i]->lData != 0x0000000055555555 || pDataArray[i]->lCount != 0)
				CCrashDump::Crash();
		}

		for (int i = 0; i < dfMAX_DATA_COUNT; i++)
		{
			if (!(g_LockFreeQueue.Enqueue(pDataArray[i])))
				CCrashDump::Crash();

			InterlockedIncrement64(&LPushCount);

		}
	}

	return 0;
}

void PrintMemoryPool(void)
{
	system("cls");

	wprintf(L"===============================================\n");
	wprintf(L"             LockFree MemoryPool Test           \n");
	wprintf(L"Thread Count : %d, Max Data Count : %d\n", 16, dfMAX_DATA_COUNT);
	wprintf(L"===============================================\n");
	wprintf(L"Memory Alloc Count : %lld, Memory Using Count : %lld, Counting : %lld\n", g_MemoryPool.GetAllocCount(), g_MemoryPool.GetUseCount(), LAllocCount);
	if (dfMAX_DATA_COUNT < g_MemoryPool.GetAllocCount())
	{
		CCrashDump::Crash();
	}
}


void InitialMemoryPool(void)
{
	for (int i = 0; i < dfMAX_DATA_COUNT; i++)
		aDataInitial[i] = g_MemoryPool.Alloc();

	for (int i = 0; i < dfMAX_DATA_COUNT; i++)
	{
		aDataInitial[i]->lCount = 0;
		aDataInitial[i]->lData = 0x0000000055555555;
	}

	for (int i = 0; i < dfMAX_DATA_COUNT; i++)
		g_MemoryPool.Free(aDataInitial[i]);
}

void InitialLockFreeStack(void)
{
	st_DATA* pDataArray[16000];

	///////////////////////////////////////////////////////////////////////////////////////////
	// 데이터 생성(확보)
	///////////////////////////////////////////////////////////////////////////////////////////
	for (int iCnt = 0; iCnt < 16000; iCnt++)
		pDataArray[iCnt] = new st_DATA;


	///////////////////////////////////////////////////////////////////////////////////////////
	// iData = 0x0000000055555555 셋팅
	// lCount = 0 셋팅
	///////////////////////////////////////////////////////////////////////////////////////////
	for (int iCnt = 0; iCnt < 16000; iCnt++)
	{
		pDataArray[iCnt]->lData = 0x0000000055555555;
		pDataArray[iCnt]->lCount = 0;
	}

	///////////////////////////////////////////////////////////////////////////////////////////
	// 2.스택에 넣음
	///////////////////////////////////////////////////////////////////////////////////////////
	for (int iCnt = 0; iCnt < 16000; iCnt++)
		g_LockFreeStack.Push(pDataArray[iCnt]);
}

void PrintLockFreeStack(void)
{
	system("cls");



	lPushTPS = LPushCount;
	lPopTPS = LPopCount;

	wprintf(L"===============================================\n");
	wprintf(L"                LockFree Stack Test            \n");
	wprintf(L"Thread Count : %d, Max Data Count : %d\n",16, dfMAX_DATA_COUNT);
	wprintf(L"===============================================\n");
	wprintf(L"Stack using count : %lld, Memory Using Count : %lld, PushTPS : %lld, PopTPS : %lld\n", g_LockFreeStack.GetUsingSize(), g_MemoryPool.GetUseCount(), LAllocCount, LPopCount);
	if (dfMAX_DATA_COUNT < g_LockFreeStack.GetUsingSize())
	{
		CCrashDump::Crash();
	}

	LPushCount = 0;
	LPopCount = 0;
}

void InitialLockFreeQueue(void)
{
	st_DATA* pDataArray[12];

	///////////////////////////////////////////////////////////////////////////////////////////
	// 데이터 생성(확보)
	///////////////////////////////////////////////////////////////////////////////////////////
	for (int iCnt = 0; iCnt <12; iCnt++)
		pDataArray[iCnt] = new st_DATA;


	///////////////////////////////////////////////////////////////////////////////////////////
	// iData = 0x0000000055555555 셋팅
	// lCount = 0 셋팅
	///////////////////////////////////////////////////////////////////////////////////////////
	for (int iCnt = 0; iCnt < 12; iCnt++)
	{
		pDataArray[iCnt]->lData = 0x0000000055555555;
		pDataArray[iCnt]->lCount = 0;
	}

	///////////////////////////////////////////////////////////////////////////////////////////
	// 2.스택에 넣음
	///////////////////////////////////////////////////////////////////////////////////////////
	for (int iCnt = 0; iCnt < 12; iCnt++)
		g_LockFreeQueue.Enqueue(pDataArray[iCnt]);

	return;
}

void PrintLockFreeQueue(void)
{
	system("cls");

	lPushTPS = LPushCount;
	lPopTPS = LPopCount;

	wprintf(L"===============================================\n");
	wprintf(L"                LockFree Queue Test            \n");
	wprintf(L"Thread Count : %d, Max Data Count : %d\n", si.dwNumberOfProcessors, si.dwNumberOfProcessors * dfMAX_DATA_COUNT);
	wprintf(L"===============================================\n");
	wprintf(L"Queue using count : %lld, Memory Using Count : %lld, PushTPS : %lld, PopTPS : %lld\n", g_LockFreeQueue.GetUsingCount(), g_LockFreeQueue.GetAllocCount(), lPushTPS, lPopTPS);
	if (si.dwNumberOfProcessors * dfMAX_DATA_COUNT < g_LockFreeQueue.GetUsingCount() || (si.dwNumberOfProcessors * dfMAX_DATA_COUNT + 1) < g_LockFreeQueue.GetAllocCount())
	{
		CCrashDump::Crash();
	}

	LPushCount = 0;
	LPopCount = 0;
}
