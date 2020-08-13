/*---------------------------------------------------------------

	procademy MemoryPool.

	메모리 풀 클래스 (오브젝트 풀 / 프리리스트)
	특정 데이타(구조체,클래스,변수)를 일정량 할당 후 나눠쓴다.

	- 사용법.

	procademy::CMemoryPool<DATA> MemPool(300, FALSE);
	DATA *pData = MemPool.Alloc();

	pData 사용

	MemPool.Free(pData);

				
----------------------------------------------------------------*/
#pragma once
#include "stdafx.h"


template <class DATA>
class CFreeList
{
private:

	/* **************************************************************** */
	// 각 블럭 앞에 사용될 노드 구조체.
	/* **************************************************************** */
	struct st_BLOCK_NODE
	{
		st_BLOCK_NODE()
		{
			stpNextBlock = nullptr;
			check = 0x1107;
			//wprintf(L"st_BLOCK_NODE Contructor\n");
		}

		~st_BLOCK_NODE()
		{
			//wprintf(L"st_BLOCK_NODE Destroyer\n");
		}
		DWORD check;
		st_BLOCK_NODE *stpNextBlock;
	};


	struct st_TOP_NODE
	{
		st_BLOCK_NODE* pTopNode;
		LONG64 lCount;
	};
public:

	//////////////////////////////////////////////////////////////////////////
	// 생성자, 파괴자.
	//
	// Parameters:	(int) 초기 블럭 개수.
	//				(bool) Alloc 시 생성자 / Free 시 파괴자 호출 여부
	// Return:
	//////////////////////////////////////////////////////////////////////////
			CFreeList(int iBlockNum, bool bPlacementNew = false);
	virtual	~CFreeList();


	//////////////////////////////////////////////////////////////////////////
	// 블럭 하나를 할당받는다.  
	//
	// Parameters: 없음.
	// Return: (DATA *) 데이타 블럭 포인터.
	//////////////////////////////////////////////////////////////////////////
	DATA	*Alloc(void);

	//////////////////////////////////////////////////////////////////////////
	// 사용중이던 블럭을 해제한다.
	//
	// Parameters: (DATA *) 블럭 포인터.
	// Return: (BOOL) TRUE, FALSE.
	//////////////////////////////////////////////////////////////////////////
	bool	Free(DATA *pData);


	//////////////////////////////////////////////////////////////////////////
	// 현재 확보 된 블럭 개수를 얻는다. (메모리풀 내부의 전체 개수)
	//
	// Parameters: 없음.
	// Return: (int) 메모리 풀 내부 전체 개수
	//////////////////////////////////////////////////////////////////////////
	int		GetAllocCount(void) { return m_iMaxCount; }

	//////////////////////////////////////////////////////////////////////////
	// 현재 사용중인 블럭 개수를 얻는다.
	//
	// Parameters: 없음.
	// Return: (int) 사용중인 블럭 개수.
	//////////////////////////////////////////////////////////////////////////
	int		GetUseCount(void) { return m_iUseCount; }


public:

private :
	// 스택 방식으로 반환된 (미사용) 오브젝트 블럭을 관리.

	st_BLOCK_NODE* _pTop;
	LONG64 m_lFreeCount;
	LONG64 m_lMaxCount;
	LONG64 m_lUseCount;
	bool  m_bUsingPlacementNew;
	LONG64 m_lCount;
};



template<class DATA>
inline CFreeList<DATA>::CFreeList(int iBlockNum, bool bPlacementNew)
{
	// 새로히 생성할 객체 블럭
	DATA* newObject = nullptr;
	st_BLOCK_NODE* newNode = nullptr;

	// 맴버 변수 초기화
	m_lMaxCount = m_lFreeCount = iBlockNum;
	m_lUseCount = 0;
	_pTop = (st_TOP_NODE*)_aligned_malloc(sizeof(st_TOP_NODE), 16);
	_pTop->pTopNode = nullptr;
	_pTop->lCount = 0;
	m_lCount = 0;
	m_bUsingPlacementNew = bPlacementNew;
	//wprintf(L"%d\n", sizeof(st_BLOCK_NODE));
	// 지역변수
	int count = m_iMaxCount;

	if (m_lMaxCount != 0)
	{

		while (count > 0)
		{
			void* newBlock = malloc(sizeof(st_BLOCK_NODE) + sizeof(DATA));
			newNode = new(newBlock) st_BLOCK_NODE;
			newNode->stpNextBlock = _pTop->pTopNode->stpNextBlock;
			_pTop->pTopNode = newNode;
			newObject = new((char*)newBlock + sizeof(st_BLOCK_NODE)) DATA;			
			count--;
			//wprintf(L"CFreeList() : MemoryPool Header Pointer : %p, newNode Pointer : %p newObject Pointer : %p\n", this->_pFreeNode->stpNextBlock, newNode, newObject);

		}
	}
}

template<class DATA>
inline CFreeList<DATA>::~CFreeList()
{
	st_BLOCK_NODE* temp;

	for (int i = 0; i < m_lMaxCount; i++)
	{
		temp = _pTop->pTopNode;
		_pTop->pTopNode = _pTop->pTopNode->stpNextBlock;
		free(temp);
	}

	_aligned_free(_pTop);
}

template<class DATA>
inline DATA* CFreeList<DATA>::Alloc(void)
{
	// 새로히 생성할 객체 블럭
	DATA* newObject = nullptr;
	st_BLOCK_NODE* newNode = nullptr;
	st_TOP_NODE CloneTop;

	LONG64 MaxCount = m_lMaxCount;
	InterlockedIncrement64(&m_lUseCount);

	// 새로 만들어야 한다면
	if (MaxCount < m_lUseCount)
	{
		newNode = (st_BLOCK_NODE*)malloc(sizeof(st_BLOCK_NODE) + sizeof(DATA));
		InterlockedIncrement64(&m_lMaxCount);
	}
	else
	{
		LONG64 newCount = InterlockedIncrement(&m_lCount);
		do
		{
			CloneTop.pTopNode = _pTop->pTopNode;
			CloneTop.lCount = _pTop->lCount;
		} while (!InterlockedCompareExchange128((LONG64*)_pTop, newCount, _pTop->pTopNode->stpNextBlock, (LONG64*)&CloneTop));

		newNode = CloneTop.pTopNode;
	}

	newObject = (DATA*)(newNode + 1);

	if (m_bUsingPlacementNew)
		new (newObject)DATA;

	return newObject;
}

template<class DATA>
inline bool CFreeList<DATA>::Free(DATA* pData)
{
	st_BLOCK_NODE* returnedBlock = (char*)pData - sizeof(st_BLOCK_NODE);
	st_TOP_NODE CloneTop;

	LONG64 newCount = InterlockedDecrement(&m_lCount);


	do
	{
		CloneTop.pTopNode = _pTop->pTopNode;
		CloneTop.lCount = _pTop->lCount;
		returnedBlock->stpNextBlock = _pTop->pTopNode;
	} while (!InterlockedCompareExchange128((LONG64*)_pTop, newCount, (LONG64)returnedBlock, (LONG64*)&CloneTop));

	InterlockedDecrement64(&m_lUseCount);

	return true;

}


