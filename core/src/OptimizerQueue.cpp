#include "OptimizerQueue.hpp"

#include <cmath>
#include <algorithm>

using namespace optimizercore;

OptimizerQueue::OptimizerQueue(int _MaxSize)
{
	MaxSize = _MaxSize;
	CurSize = 0;
	pMem = new OptimizerInterval[MaxSize];
}

// ------------------------------------------------------------------------------------------------
/*optimizercore::OptimizerQueue::~OptimizerQueue()
{
	delete[] pMem;
}
*/
// ------------------------------------------------------------------------------------------------
bool OptimizerQueue::IsEmpty() const
{
	return CurSize == 0;
}
// ------------------------------------------------------------------------------------------------
int OptimizerQueue::GetSize() const
{
	return CurSize;
}

// ------------------------------------------------------------------------------------------------
bool OptimizerQueue::IsFull() const
{
	return CurSize == MaxSize;
}

// ------------------------------------------------------------------------------------------------
void OptimizerQueue::Push(const OptimizerInterval &value)
{
	if (IsFull())
	{
		int MinInd = GetIndOfMinElem();
		if (value.R > pMem[MinInd].R)
			DeleteMinElem();
		else
			return;
	}
	CurSize++;
	pMem[CurSize - 1] = value;
	if (CurSize > 1)
		ReBuild(CurSize - 1);
}

// ------------------------------------------------------------------------------------------------
void OptimizerQueue::PushWithPriority(const OptimizerInterval &value)
{
	if (IsEmpty())
	{
		CurSize++;
		pMem[CurSize - 1] = value;
	}
	else
	{
		int MinInd = GetIndOfMinElem();

		// � ������� ������ ���� �������� � ����������� ����������������!!!
		if (value.R >= pMem[MinInd].R)
		{
			if (IsFull())
				DeleteMinElem();
			CurSize++;
			pMem[CurSize - 1] = value;
			if (CurSize > 1)
				ReBuild(CurSize - 1);
		}
	}
}

void optimizercore::OptimizerQueue::DeleteInterval(const OptimizerInterval & value)
{
	for (int i = 0; i < CurSize; i++)
		if (pMem[i].right.x == value.right.x)
		{
			pMem[i] = pMem[CurSize - 1];
			CurSize--;
			if (CurSize > 1)
				ReBuild(i);
			break;
		}
}

// ------------------------------------------------------------------------------------------------
OptimizerInterval OptimizerQueue::Pop()
{
	OptimizerInterval tmp = pMem[0];
	pMem[0] = pMem[CurSize - 1];
	CurSize--;
	if (CurSize > 1)
		ReBuild(0);

	return tmp;
}

// ------------------------------------------------------------------------------------------------
int OptimizerQueue::GetIndOfMinElem()
{
	int i, StartIndex;
	double min = HUGE_VAL;
	int MinIndex = -1;

	// ������� ����� ������� ��� ������ ��������. ����� ������� - ������ ���� � ����.
	// ���� ��������� ������� ������ �������� �� �� �����, �� ������ ���� � ������������� ������.
	// ���� � ���� ���� �������, �� � ���� ���� �� ����� ���� �������� (�� �������� d-����)
	if (CurSize % 2) // ������� CurSize - ����� ������� ������ ������
		StartIndex = (CurSize - 1) / 2; // ������ ������� ���������� ����� � ���� - ������� ������
	else // ������� CurSize - ������ ������� ������ ������
		StartIndex = (CurSize - 1) / 2 + 1; // ������� ������ - ��������� ������ �� ������� CurSize

	for (i = StartIndex; i < CurSize; i++)
		if (min > pMem[i].R)
		{
			MinIndex = i;
			min = pMem[i].R;
		}

	return MinIndex;
}

// ------------------------------------------------------------------------------------------------
void OptimizerQueue::DeleteMinElem()
{
	int MinInd = GetIndOfMinElem();
	pMem[MinInd] = pMem[CurSize - 1];
	CurSize--;
	if (CurSize > 1)
		ReBuild(MinInd);
}

// ------------------------------------------------------------------------------------------------
void OptimizerQueue::ReBuild(int Index)
{
	int i, j, k, level = 0;
	if (Index == 0) // �������������� ��������� �������� ���� �� ����� (����������)
	{
		i = Index;
		j = 2 * i + 1; // ����� ������� ����� (������ ����������)
		k = 2 * i + 2; // ������ ������� ����� (����� �������������)
	//    if (k < CurSize - 1) // ��� ������� ����������
		if (k < CurSize) // ��� ������� ����������
			if (pMem[j].R < pMem[k].R)
				j = k;
		// ������ j - ���������� ������� ���� i
		while (true)
		{
			if (pMem[i].R >= pMem[j].R)
				break;
			std::swap(pMem[i], pMem[j]);

			i = j;
			j = 2 * i + 1;
			k = 2 * i + 2;
			if (j > CurSize - 1) // � ���� i ��� ��������
				break;
			//      if (k < CurSize - 1) // ��� ������� ����������
			if (k < CurSize) // ��� ������� ����������
				if (pMem[j].R < pMem[k].R)
					j = k;
		}
	}
	else // �������������� ��������� �������� ���� �� ����� (��������)
	{
		i = Index;
		j = (i - 1) / 2; // ������ ���� i
		while ((i > 0) && (pMem[j].R <= pMem[i].R)) //� ����� �� ��� ��������� <= ������ < ?
		{
			std::swap(pMem[i], pMem[j]);
			i = j;
			j = (i - 1) / 2;
		}
	}
}

void OptimizerQueue::Clear()
{
	CurSize = 0;
}
// - end of file ----------------------------------------------------------------------------------
