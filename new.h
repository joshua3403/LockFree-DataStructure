﻿#pragma once
#include "stdafx.h"

/*--------------------------------------------------------------------------*/
// 락프리 메모리 풀
//
// 락 없이 다중 쓰레드 환경에 안전하게 메모리 블럭을 할당, 해제 해준다
/*--------------------------------------------------------------------------*/
template <class DATA>
class CMemoryPool
{
private:

	/* ******************************************************************** */
	// 각 블럭 앞에 사용될 노드 구조체.
	/* ******************************************************************** */
	struct st_BLOCK_NODE
	{
		st_BLOCK_NODE()
		{
			stpNextBlock = NULL;
		}

		st_BLOCK_NODE* stpNextBlock;
	};

	/* ******************************************************************** */
	// 락프리 메모리풀 탑 노드.
	/* ******************************************************************** */
	struct st_TOP_NODE
	{
		st_BLOCK_NODE* pTopNode;
		__int64			iUniqueNum;
	};


public:

	//////////////////////////////////////////////////////////////////////////
	// 생성자, 파괴자.
	//
	// Parameters:	(int) 최대 블럭 개수.
	//				(bool) 메모리 Lock 플래그 - 중요하게 속도를 필요로 한다면 Lock.
	// Return:
	//////////////////////////////////////////////////////////////////////////
	CMemoryPool(bool bLockFlag = false)
	{
		//////////////////////////////////////////////////////////////////////
		// 초기화
		//////////////////////////////////////////////////////////////////////
		_lBlockCount = 0;
		_lAllocCount = 0;
		_bLockflag = bLockFlag;

		//////////////////////////////////////////////////////////////////////
		// 탑 노드 할당
		//////////////////////////////////////////////////////////////////////
		_pTop = (st_TOP_NODE*)_aligned_malloc(sizeof(st_TOP_NODE), 16);
		_pTop->pTopNode = nullptr;
		_pTop->iUniqueNum = 0;

		_iUniqueNum = 0;
	}

	virtual					~CMemoryPool()
	{
		st_BLOCK_NODE* pNode;

		for (int iCnt = 0; iCnt < _lBlockCount; iCnt++)
		{
			pNode = _pTop->pTopNode;
			_pTop->pTopNode = _pTop->pTopNode->stpNextBlock;
			free(pNode);
		}

		_aligned_free(_pTop);
	}


	//////////////////////////////////////////////////////////////////////////
	// 블럭 하나를 할당받는다.
	//
	// Parameters: 없음.
	// Return: (DATA *) 데이타 블럭 포인터.
	//////////////////////////////////////////////////////////////////////////
	DATA* Alloc(bool bPlacementNew = true)
	{
		st_BLOCK_NODE* pAllocNode = nullptr;
		st_TOP_NODE		stCloneTopNode;

		DATA* pData;

		long			lBlockCount = _lBlockCount;
		InterlockedIncrement((long*)&_lAllocCount);

		//////////////////////////////////////////////////////////////////////
		// 할당 해야 할 경우
		//////////////////////////////////////////////////////////////////////
		if (lBlockCount < _lAllocCount)
		{
			pAllocNode = (st_BLOCK_NODE*)malloc(sizeof(st_BLOCK_NODE) + sizeof(DATA));
			InterlockedIncrement((long*)&_lBlockCount);
		}

		else
		{
			__int64 iUniqueNum = InterlockedIncrement((long*)&_iUniqueNum);

			do
			{
				stCloneTopNode.iUniqueNum = _pTop->iUniqueNum;
				stCloneTopNode.pTopNode = _pTop->pTopNode;
			} while (!InterlockedCompareExchange128(
				(LONG64*)_pTop,
				iUniqueNum,
				(LONG64)_pTop->pTopNode->stpNextBlock,
				(LONG64*)&stCloneTopNode
			));

			pAllocNode = stCloneTopNode.pTopNode;
		}

		pData = (DATA*)(pAllocNode + 1);

		if (bPlacementNew)
			new (pData)DATA;

		return pData;
	}

	//////////////////////////////////////////////////////////////////////////
	// 사용중이던 블럭을 해제한다.
	//
	// Parameters: (DATA *) 블럭 포인터.
	// Return: (BOOL) TRUE, FALSE.
	//////////////////////////////////////////////////////////////////////////
	bool					Free(DATA* pData)
	{
		st_BLOCK_NODE* pReturnNode = ((st_BLOCK_NODE*)pData) - 1;
		st_TOP_NODE		stCloneTopNode;

		__int64			iUniqueNum = InterlockedIncrement((long*)&_iUniqueNum);

		do
		{
			stCloneTopNode.iUniqueNum = _pTop->iUniqueNum;
			stCloneTopNode.pTopNode = _pTop->pTopNode;

			pReturnNode->stpNextBlock = _pTop->pTopNode;
		} while (!InterlockedCompareExchange128(
			(LONG64*)_pTop,
			iUniqueNum,
			(LONG64)pReturnNode,
			(LONG64*)&stCloneTopNode
		));

		InterlockedDecrement((long*)&_lAllocCount);

		return true;
	}


	//////////////////////////////////////////////////////////////////////////
	// 현재 사용중인 블럭 개수를 얻는다.
	//
	// Parameters: 없음.
	// Return: (int) 사용중인 블럭 개수.
	//////////////////////////////////////////////////////////////////////////
	int						GetAllocCount(void) { return (int)_lAllocCount; }


	//////////////////////////////////////////////////////////////////////////
	// MemoryPool의 전체 블럭 갯수.
	//
	// Parameters: 없음
	// Return: (int) 전체 블럭 갯수.
	//////////////////////////////////////////////////////////////////////////
	int						GetBlockCount(void) { return (int)_lBlockCount; }



private:
	//////////////////////////////////////////////////////////////////////////
	// MemoryPool의 Top
	//////////////////////////////////////////////////////////////////////////
	st_TOP_NODE* _pTop;


	//////////////////////////////////////////////////////////////////////////
	// Top의 Unique Number
	//////////////////////////////////////////////////////////////////////////
	__int64					_iUniqueNum;


	//////////////////////////////////////////////////////////////////////////
	// Lockflag
	//////////////////////////////////////////////////////////////////////////
	bool					_bLockflag;


	//////////////////////////////////////////////////////////////////////////
	// 현재 할당중인 블록 수
	//////////////////////////////////////////////////////////////////////////
	long					_lAllocCount;


	//////////////////////////////////////////////////////////////////////////
	// 전체 블록 수
	//////////////////////////////////////////////////////////////////////////
	long					_lBlockCount;
};


