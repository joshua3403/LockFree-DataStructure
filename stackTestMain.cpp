#include "Stack(LockFree).h"

int main()
{

	CLockFreeStack test;

	CLockFreeStack::st_NODE* newNode = test.CreateNode(10, UNIQUENUM);
	test.Push(newNode);


	//CLockFreeStack<LONG64>::st_TOP_NODE first = test.Pop();
	//wprintf(L"data : %lld\n", first.pTopNode->Data);
	//CLockFreeStack<LONG64>::st_TOP_NODE second = test.Pop();
	//wprintf(L"data : %lld\n", second.pTopNode->Data);

	delete newNode;

	return 0;
}