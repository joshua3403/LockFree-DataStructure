#pragma once
#include <stdio.h>
#include <Windows.h>
#include <winnt.h>
#include <atomic>
#include <vector>
#include <process.h>

struct __declspec(align(16)) st_NODE
{
	LONG64 lData[2];
	st_NODE* pNextNode;

	st_NODE(LONG64 data, LONG64 count)
	{
		lData[0] = data;
		lData[1] = count;
		pNextNode = nullptr;
	}
};


class CStack
{
private:


private:
	LONG64 _lUsingCount;

	std::atomic<st_NODE*> _pTop = nullptr;
public:

	void Push(LONG64 data)
	{
		st_NODE* temp_node = new st_NODE(data, 10);
		st_NODE* temp;
		do
		{
			temp = _pTop.load();
			temp_node->pNextNode = temp;
		} while (!std::atomic_compare_exchange_weak(&_pTop,&temp,temp_node));

	}

	LONG64 Pop()
	{

	}

	
};