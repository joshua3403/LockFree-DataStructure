#include "Stack(LockFree).h"

unsigned int WINAPI ThreadFunc(VOID* arg);

CLockFreeStack<LONG64> stack;

int main()
{
	std::vector<HANDLE> threadhandle;
	SYSTEM_INFO si;

	GetSystemInfo(&si);

	DWORD dwThreadID = 0;
	DWORD t = 0;
	for (int i = 0; i < si.dwNumberOfProcessors; i++)
	{
		threadhandle.push_back((HANDLE)_beginthreadex(NULL, 0, ThreadFunc, &t, 0, (unsigned int*)&dwThreadID));
		t += 10000;
		CloseHandle(threadhandle[i]);
	}
	return 0;
}

unsigned int __stdcall ThreadFunc(VOID* arg)
{
	srand(GetCurrentThreadId());
	DWORD input = *(DWORD*)arg;
	for (int i = 0; i < 10000; i++)
	{
		stack.Push(input++);
	}

	Sleep(0);

	for (int i = 0; i < 10000; i++)
	{
		CLockFreeStack<LONG64>::st_NODE* newNode = stack.Pop();

		if (newNode->Data != 0x0000000055555555)
		{
			// crash
		}

		InterlockedIncrement64(&newNode->lData[0]);
		InterlockedIncrement64(&newNode->lData[1]);

		Sleep(0);


	}


	return 0;
}
