/*---------------------------------------------------------------

	procademy MemoryPool.

	�޸� Ǯ Ŭ���� (������Ʈ Ǯ / ��������Ʈ)
	Ư�� ����Ÿ(����ü,Ŭ����,����)�� ������ �Ҵ� �� ��������.

	- ����.

	procademy::CMemoryPool<DATA> MemPool(300, FALSE);
	DATA *pData = MemPool.Alloc();

	pData ���

	MemPool.Free(pData);

				
----------------------------------------------------------------*/
#pragma once
#include "stdafx.h"


template <class DATA>
class CFreeList
{
private:

	/* **************************************************************** */
	// �� �� �տ� ���� ��� ����ü.
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
	// ������, �ı���.
	//
	// Parameters:	(int) �ʱ� �� ����.
	//				(bool) Alloc �� ������ / Free �� �ı��� ȣ�� ����
	// Return:
	//////////////////////////////////////////////////////////////////////////
			CFreeList(int iBlockNum, bool bPlacementNew = false);
	virtual	~CFreeList();


	//////////////////////////////////////////////////////////////////////////
	// �� �ϳ��� �Ҵ�޴´�.  
	//
	// Parameters: ����.
	// Return: (DATA *) ����Ÿ �� ������.
	//////////////////////////////////////////////////////////////////////////
	DATA	*Alloc(void);

	//////////////////////////////////////////////////////////////////////////
	// ������̴� ���� �����Ѵ�.
	//
	// Parameters: (DATA *) �� ������.
	// Return: (BOOL) TRUE, FALSE.
	//////////////////////////////////////////////////////////////////////////
	bool	Free(DATA *pData);


	//////////////////////////////////////////////////////////////////////////
	// ���� Ȯ�� �� �� ������ ��´�. (�޸�Ǯ ������ ��ü ����)
	//
	// Parameters: ����.
	// Return: (int) �޸� Ǯ ���� ��ü ����
	//////////////////////////////////////////////////////////////////////////
	int		GetAllocCount(void) { return m_iMaxCount; }

	//////////////////////////////////////////////////////////////////////////
	// ���� ������� �� ������ ��´�.
	//
	// Parameters: ����.
	// Return: (int) ������� �� ����.
	//////////////////////////////////////////////////////////////////////////
	int		GetUseCount(void) { return m_iUseCount; }


public:

private :
	// ���� ������� ��ȯ�� (�̻��) ������Ʈ ���� ����.

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
	// ������ ������ ��ü ��
	DATA* newObject = nullptr;
	st_BLOCK_NODE* newNode = nullptr;

	// �ɹ� ���� �ʱ�ȭ
	m_lMaxCount = m_lFreeCount = iBlockNum;
	m_lUseCount = 0;
	_pTop = (st_TOP_NODE*)_aligned_malloc(sizeof(st_TOP_NODE), 16);
	_pTop->pTopNode = nullptr;
	_pTop->lCount = 0;
	m_lCount = 0;
	m_bUsingPlacementNew = bPlacementNew;
	//wprintf(L"%d\n", sizeof(st_BLOCK_NODE));
	// ��������
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
	// ������ ������ ��ü ��
	DATA* newObject = nullptr;
	st_BLOCK_NODE* newNode = nullptr;
	st_TOP_NODE CloneTop;

	LONG64 MaxCount = m_lMaxCount;
	InterlockedIncrement64(&m_lUseCount);

	// ���� ������ �Ѵٸ�
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


