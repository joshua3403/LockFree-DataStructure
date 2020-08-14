#include "stdafx.h"
#include "Stack(LockFree).h"
#include "MemoryPool_mail.h"
#include "CCrashDumpClass.h"

#define dfMAX_DATA_COUNT 4000

struct st_DATA
{
	LONG64 lData;
	LONG64 lCount;
};

void InitialMemoryPool(void);

BOOL bShutDown = FALSE;
LONG64 LAllocCount = 0;
CFreeList<st_DATA> g_MemoryPool(0, FALSE);
st_DATA* aDataInitial[dfMAX_DATA_COUNT];

unsigned int WINAPI ThreadFunc(VOID* arg);

int main()
{
	CCrashDump::CCrashDump();
	std::vector<HANDLE> threadhandle;
	SYSTEM_INFO si;

	GetSystemInfo(&si);



	DWORD dwThreadID = 0;
	DWORD t = 0;

	InitialMemoryPool();

	for (int i = 0; i < si.dwNumberOfProcessors; i++)
	{
		threadhandle.push_back((HANDLE)_beginthreadex(NULL, 0, ThreadFunc, &t, 0, (unsigned int*)&dwThreadID));
		t += 10000;
		CloseHandle(threadhandle[i]);
	}

	wprintf(L"===============================================\n");
	wprintf(L"             LocFree MemoryPool Test           \n");
	wprintf(L"Thread Count : %d, Max Data Count : %d\n", si.dwNumberOfProcessors, dfMAX_DATA_COUNT);
	wprintf(L"===============================================\n");

	while (true)
	{
		system("cls");
		if (GetAsyncKeyState('q') & 0x8000 || GetAsyncKeyState('Q') & 0x8000)
		{
			break;
		}

		wprintf(L"Alloc Count : %lld, Using Count : %lld\n", g_MemoryPool.GetAllocCount(), g_MemoryPool.GetUseCount());
		if (LAllocCount != g_MemoryPool.GetAllocCount())
		{
			wprintf(L"Error! LAlloc Count : %lld, MemoryPool Alloc Count : %lld\n", LAllocCount, g_MemoryPool.GetAllocCount());
			CCrashDump::Crash();
		}

		Sleep(999);
		
	}
	return 0;
}

unsigned int __stdcall ThreadFunc(VOID* arg)
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
			if (aTestData[i]->lData != 0x0000000055555555 || aTestData[i]->lCount
				!= 0)
				CCrashDump::Crash();
		}

		for (int i = 0; i < 1000; i++)
		{
			InterlockedIncrement64(&aTestData[i]->lData);
			InterlockedIncrement64(&aTestData[i]->lCount);
		}

		for (int i = 0; i < 1000; i++)
		{
			if (aTestData[i]->lData != 0x0000000055555556 || aTestData[i]->lCount
				!= 1)
				CCrashDump::Crash();
		}

		for (int i = 0; i < 1000; i++)
		{
			InterlockedDecrement64(&aTestData[i]->lData);
			InterlockedDecrement64(&aTestData[i]->lCount);
		}

		for (int i = 0; i < 1000; i++)
		{
			if (aTestData[i]->lData != 0x0000000055555555 || aTestData[i]->lCount
				!= 0)
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