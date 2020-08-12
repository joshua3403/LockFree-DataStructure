#pragma once
#include <stdio.h>
#include <Windows.h>
#include <winnt.h>
#include <atomic>
#include <vector>
#include <process.h>

#define UNIQUENUM 0x0000000055555555

class CLockFreeStack
{
public:
	struct st_NODE;

	struct st_DATA
	{
		LONG64 data;
		st_NODE* pNext;
	};

	struct __declspec(align(128))st_NODE
	{
		st_DATA Data;
		LONG64 lCount;
	};


public:
	CLockFreeStack()
	{
		_lUsingsize = 0;
		_lCount = 0;
		_pTop = (st_NODE*)_aligned_malloc(sizeof(st_NODE), 16);
		_pTop->Data.data = 0;
		_pTop->Data.pNext = nullptr;
		_pTop->lCount = 0;

	}

	void Push(st_NODE* newNode)
	{
		st_NODE stClone;
		do
		{
			stClone = _pTop;
			newNode->Data.pNext = stClone;
		} while (!InterlockedCompareExchange128((LONG64*)&_pTop, newNode->Data.data, (LONG64)UNIQUENUM, (LONG64*)&stClone));

		InterlockedIncrement64(&_lUsingsize);
	}

	st_NODE* CreateNode(LONG64 data, LONG64 count)
	{
		st_NODE* newNode = (st_NODE*)_aligned_malloc(sizeof(st_NODE), 16);
		newNode->Data.data = data;
		newNode->lCount = count;
		newNode->Data.pNext = nullptr;
		return newNode;
	}

	//BOOL Pop(LONG64* outData, LONG64* outCount)
	//{
	//	st_TOP_NODE stClone;
	//	
	//	if (isEmpty())
	//		return FALSE;

	//	InterlockedDecrement64(&_lUsingsize);

	//	do
	//	{
	//		stClone.pTopNode = _pTop->pTopNode;
	//		stClone.lCount = _pTop->lCount;
	//	} while (!InterlockedCompareExchange128((LONG64*)_pTop, UNIQUENUM, (LONG64)_pTop->pTopNode->pNext, (LONG64*)&stClone));

	//	return TRUE;
	//}

	BOOL isEmpty()
	{
		if (_lUsingsize == 0)
		{
			if (_pTop->Data.pNext == nullptr)
				return TRUE;
		}

		return FALSE;
	}


private:

	st_NODE* _pTop;

	LONG64 _lCount;

	LONG64 _lUsingsize;
};