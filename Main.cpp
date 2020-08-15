#include "stdafx.h"
#include "CCrashDumpClass.h"

#define dfMAX_DATA_COUNT 400

struct st_DATA
{
	LONG64 lData;
	LONG64 lCount;
};

void InitialMemoryPool(void);

BOOL bShutDown = FALSE;
LONG64 LAllocCount = 0;
CFreeList<st_DATA> g_MemoryPool(0,FALSE);
st_DATA* aDataInitial[dfMAX_DATA_COUNT];
SYSTEM_INFO si;


unsigned int WINAPI ThreadFunc(VOID* arg);

int main()
{
	CCrashDump::CCrashDump();
	std::vector<HANDLE> threadhandle;
	GetSystemInfo(&si);


	DWORD dwThreadID = 0;
	DWORD t = 0;

	InitialMemoryPool();

	for (DWORD i = 0; i < si.dwNumberOfProcessors; i++)
	{
		threadhandle.push_back((HANDLE)_beginthreadex(NULL, 0, ThreadFunc, &t, 0, (unsigned int*)&dwThreadID));
		t += 10000;
		CloseHandle(threadhandle[i]);
	}

	while (true)
	{
		system("cls");
		if (GetAsyncKeyState('q') & 0x8000 || GetAsyncKeyState('Q') & 0x8000)
		{
			bShutDown = true;
			break;
		}
		wprintf(L"===============================================\n");
		wprintf(L"             LocFree MemoryPool Test           \n");
		wprintf(L"Thread Count : %d, Max Data Count : %d\n", si.dwNumberOfProcessors, dfMAX_DATA_COUNT);
		wprintf(L"===============================================\n");
		wprintf(L"Memory Alloc Count : %lld, Memory Using Count : %lld, Counting : %lld\n", g_MemoryPool.GetAllocCount(), g_MemoryPool.GetUseCount(), LAllocCount);
		if (dfMAX_DATA_COUNT < g_MemoryPool.GetAllocCount())
		{
				CCrashDump::Crash();
		}

		Sleep(999);
		
	}

	wprintf(L"Stoped\n");
	return 0;
}

unsigned int __stdcall ThreadFunc(VOID* arg)
{
	st_DATA* aTestData[100];
	while (!bShutDown)
	{
		for (int i = 0; i < 100; i++)
		{
			aTestData[i] = g_MemoryPool.Alloc();
			InterlockedIncrement64(&LAllocCount);
		}

		for (int i = 0; i < 100; i++)
		{
			if ((aTestData[i]->lData != 0x0000000055555555) || (aTestData[i]->lCount
				!= 0))
				CCrashDump::Crash();
		}

		for (int i = 0; i < 100; i++)
		{
			InterlockedIncrement64(&aTestData[i]->lData);
			InterlockedIncrement64(&aTestData[i]->lCount);
		}

		for (int i = 0; i < 100; i++)
		{
			if ((aTestData[i]->lData != 0x0000000055555556) || (aTestData[i]->lCount
				!= 1))
				CCrashDump::Crash();
		}

		for (int i = 0; i < 10; i++)
		{
			InterlockedDecrement64(&aTestData[i]->lData);
			InterlockedDecrement64(&aTestData[i]->lCount);
		}

		for (int i = 0; i < 100; i++)
		{
			if ((aTestData[i]->lData != 0x0000000055555555) || (aTestData[i]->lCount
				!= 0))
				CCrashDump::Crash();
		}

		for (int i = 0; i < 100; i++)
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