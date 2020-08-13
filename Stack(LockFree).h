#pragma once
#include "stdafx.h"


template<typename DATA>
class CLockFreeStack
{
public:
	struct st_NODE
	{
		DATA data;
		st_NODE* pNextNode;

		st_NODE()
		{
			wprintf(L"Node Created\n");
		}

		~st_NODE()
		{
			wprintf(L"Node Deleted\n");
		}
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

	virtual ~CLockFreeStack()
	{
		st_NODE* pDeleteNode = nullptr;
		for (int i = 0; i < _lUsingsize; i++)
		{
			pDeleteNode = _pTop->pTopNode;
			_pTop->pTopNode = _pTop->pTopNode->pNextNode;
			delete (pDeleteNode);
		}

		_aligned_free(_pTop);

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
		st_TOP_NODE stClone;

		if (isEmpty() == TRUE )
		{
			return nullptr;
		}


		InterlockedDecrement64(&_lUsingsize);

		LONG64 newCount = InterlockedIncrement64(&_lCount);

		do
		{
			stClone.pTopNode = _pTop->pTopNode;
			stClone.lCount = _pTop->lCount;
		} while (!InterlockedCompareExchange128((LONG64*)_pTop, newCount, (LONG64)_pTop->pTopNode->pNextNode, (LONG64*)&stClone));

		DATA* temp = &stClone.pTopNode->data;

		//delete stClone.pTopNode;
		return temp;
	}

	BOOL isEmpty()
	{
		if (_lUsingsize == 0)
		{
			if (_pTop->pTopNode->pNextNode == nullptr)
				return TRUE;
		}

		return FALSE;
	}

	LONG64 GetUsingSize()
	{
		return _lUsingsize;
	}

private:

	LONG64 _lCount;

	st_TOP_NODE* _pTop;

	LONG64 _lUsingsize;
};