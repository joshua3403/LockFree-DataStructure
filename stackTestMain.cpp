#include "stdafx.h"
#include "Stack(LockFree).h"

struct st_NODE
{
	LONG64 lData;
	LONG64 lCount;
};

int main()
{

	CLockFreeStack<st_NODE> test;

	st_NODE testdata;
	testdata.lData = 10000;
	testdata.lCount = 1234;
	test.Push(testdata);
	testdata.lData = 20000;
	testdata.lCount = 1234;
	test.Push(testdata);

	st_NODE* testptr = test.Pop();


	wprintf(L"%lld\n", testptr->lData);

	//CLockFreeStack<LONG64>::st_TOP_NODE first = test.Pop();
	//wprintf(L"data : %lld\n", first.pTopNode->Data);
	//CLockFreeStack<LONG64>::st_TOP_NODE second = test.Pop();
	//wprintf(L"data : %lld\n", second.pTopNode->Data);

	return 0;
}