#pragma once
#include "stdafx.h"
#include "MemoryPool(LockFree).h"

template <typename DATA>
class CQueue
{
private:
	LONG64 m_lSize;


	struct st_NODE
	{
		DATA data;
		st_NODE* NextNode;
	};

	struct st_TOP_NODE
	{
		st_NODE* pTopNode;
		LONG64 lCount;
	};

	st_TOP_NODE* m_pHead;
	st_TOP_NODE* m_pTail;

	CFreeList<st_NODE>* m_MemoryPool;

	st_NODE* m_pDummyNode;

	LONG64 m_lHeadCount;
	LONG64 m_lTailCount;
public:
	CQueue()
	{
		m_lSize = m_lHeadCount = m_lTailCount= 0;
		m_MemoryPool = new CFreeList<st_NODE>(0, FALSE);

		m_pDummyNode = m_MemoryPool->Alloc();
		m_pDummyNode->data = 0;
		m_pDummyNode->NextNode = nullptr;

		m_pHead = (st_TOP_NODE*)_aligned_malloc(sizeof(st_TOP_NODE), 16);
		m_pHead->lCount = 0;
		m_pHead->pTopNode = m_pDummyNode;

		m_pTail = (st_TOP_NODE*)_aligned_malloc(sizeof(st_TOP_NODE), 16);
		m_pTail->lCount = 0;
		m_pTail->pTopNode = m_pDummyNode;
	}

	~CQueue()
	{
		st_NODE* temp;
		while (m_pHead->pTopNode != nullptr)
		{
			temp = m_pHead->pTopNode;
			m_pHead->pTopNode = m_pHead->pTopNode->NextNode;
			m_MemoryPool->Free(temp);
		}

		delete m_MemoryPool;

		_aligned_free(m_pHead);
		_aligned_free(m_pTail);

	}

	BOOL Enqueue(DATA data)
	{
		st_NODE* newNode = m_MemoryPool->Alloc();
		newNode->data = data;
		newNode->NextNode = nullptr;

		st_TOP_NODE stCloneNode;
		st_NODE* nextNode;

		while(true)
		{
			LONG64 newCount = InterlockedIncrement64(&m_lTailCount);

			stCloneNode.pTopNode = m_pTail->pTopNode;
			stCloneNode.lCount = m_pTail->lCount;
			nextNode = m_pTail->pTopNode->NextNode;

			if (nextNode == nullptr)
			{
				// tail�� next�� null�̰� ���� �̸� ���� tail�� ������ ��尡 ����Ű�� ���� ��� �ּҰ��� tail�� ����Ű�� �ִ� ����� ���� ��� �ּҰ��� ���� �̰� nullptr�̸�.
				if (_InterlockedCompareExchangePointer((PVOID*)&stCloneNode.pTopNode->NextNode, newNode, nextNode) == nullptr)
				{
					InterlockedCompareExchange128((LONG64*)m_pTail, newCount, (LONG64)stCloneNode.pTopNode->NextNode, (LONG64*)&stCloneNode);
					break;
				}
			}
			// �ٸ� �����忡�� enqeueue�� tail�� ��� ���� �ܰ踦 �����ߴٸ�.
			// �׳� ������ �Ű��ְ� ���� �õ� ����.
			else
			{
				InterlockedCompareExchange128((LONG64*)m_pTail, newCount, (LONG64)stCloneNode.pTopNode->NextNode, (LONG64*)&stCloneNode);
			}
		}

		InterlockedIncrement64(&m_lSize);

		return TRUE;
	}

	BOOL Dequeue(DATA* data)
	{
		st_TOP_NODE stCloneHeadNode, stCloneTailNode;
		st_NODE* nextNode;

		LONG64 newHead = InterlockedIncrement64(&m_lHeadCount);

		InterlockedDecrement64(&m_lSize);

		while (true)
		{
			// ��� ����
			stCloneHeadNode.pTopNode = m_pHead->pTopNode;
			stCloneHeadNode.lCount = m_pHead->lCount;

			// ���� ����
			stCloneTailNode.pTopNode = m_pTail->pTopNode;
			stCloneTailNode.lCount = m_pTail->lCount;

			// ����� next����
			nextNode = stCloneHeadNode.pTopNode->NextNode;

			// ����ٸ�
			if (m_lSize == 0)
			{
				//if (m_pHead->pTopNode == m_pTail->pTopNode)
				//{

				//}
				data = nullptr;
				return FALSE;
			}
			//else if (nullptr != stCloneTailNode.pTopNode->NextNode)
			//{
			//	LONG64 newTail = InterlockedIncrement64(&m_lTailCount);
			//	InterlockedCompareExchange128((LONG64*)m_pTail, newTail, (LONG64)stCloneTailNode.pTopNode->NextNode, (LONG64*)&stCloneTailNode);

			//}
			else
			{

				// ����� next�� ��尡 �����Ѵٸ�
				if (nextNode != nullptr)
				{
					*data = nextNode->data;
					if (InterlockedCompareExchange128((LONG64*)m_pHead, newHead, (LONG64)stCloneHeadNode.pTopNode->NextNode, (LONG64*)&stCloneHeadNode))
					{
						m_MemoryPool->Free(stCloneHeadNode.pTopNode);
						break;
					}
				}
			}
		}

		return TRUE;
	}

	LONG64 GetUsingCount(void)
	{
		return m_lSize;
	}

	LONG64 GetAllocCount(void)
	{
		return m_MemoryPool->GetAllocCount();
	}
};




// Enqeueue
// ������ �ܰ�� 2�ܰ�
// tail�ڿ� �� ��� ����
// tail�� �� ���� ����

// tail�ڿ� �� ��带 �����ϴ� ���߿� �ٸ� �����忡�� enqeueue�� �õ�. tail�� next���� �� ��尡 �ִ� ����.
// �ٸ� ������� tail�� next�� Ȯ�������� tail�� �ű�� 2��° cas�� �������� �ʾƼ� tail�� �� ��带 ���� �� �� ����
// �׷��� �ٸ� �����尡 tail�� ��ĭ �ű�� �ٽ� tail�� next�� �ٽ� Ȯ������.
// ���� ������� 2��° cas�� �����ϴ� �����ϴ� ������� �����ϰ� �������´�.
// tail�� ��ġ�� ��¥�� �ٸ� �����忡�� �Ű��� ���̱� ����.