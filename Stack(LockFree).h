#pragma once
#include <stdio.h>
#include <Windows.h>
#include <winnt.h>
#include <atomic>
#include <vector>
#include <process.h>


template<typename DATA>
class CLockFreeStack
{
public:
	typedef struct st_NODE
	{
		DATA data;
		st_NODE* pNextNode;
	};

	struct st_TOP_NODE
	{
		st_NODE* pTopNode;
		LONG64 lCount;
	};

public:

	CLockFreeStack()
	{
		_pTop = (st_TOP_NODE*)_aligned_malloc(sizeof(st_TOP_NODE), 16);
		_pTop->pTopNode = nullptr;
		_pTop->lCount = 0;
		_lUsingsize = 0;
	}

	// 템플릿 데이터 자체를 받아야 한다.
	BOOL Push(DATA newData)
	{
		st_TOP_NODE stClone;
		st_NODE* temp = new st_NODE();
		temp->data = newData;

		LONG64 newCount = InterlockedIncrement64(&_lCount);

		do
		{
			stClone.pTopNode = _pTop->pTopNode;
			stClone.lCount = _pTop->lCount;
			temp->pNextNode = _pTop->pTopNode;
		} while (!InterlockedCompareExchange128((LONG64*)_pTop, newCount, (LONG64)temp, (LONG64*)&stClone));

		InterlockedIncrement64(&_lUsingsize);

		return TRUE;
	}

	DATA* Pop()
	{

	}

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

	LONG64 _lCount;

	st_TOP_NODE* _pTop;

	LONG64 _lUsingsize;
};