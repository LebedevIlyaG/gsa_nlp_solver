#include "OptimizerMultiMap.hpp"
#include "CoreUtils.hpp"
#include "Map.hpp"

#include <cassert>
#include <cmath>

optimizercore::OptimizerMultiMap::OptimizerMultiMap()
{
	mIsInitialized = false;
}

optimizercore::OptimizerMultiMap::OptimizerMultiMap(MultimapType mapType, int n, int m, int l)
{
	assert(n > 1);
	assert(m > 1);
	assert(l > 0);

	mDimension = n;
	mTightness = m;
	mNumberOfMaps = l;
	mMapType = mapType;

	if (mapType == MultimapType::Rotated)	{
		assert(l <= n*(n-1));
		InitRotatedMap();
	}
	else if (mapType == MultimapType::Noninjective)
		mNumberOfMaps = 1;

	mIsInitialized = true;
}

optimizercore::OptimizerMultiMap::~OptimizerMultiMap()
{
	if (mMapType == MultimapType::Rotated && mIsInitialized)
	{
		int PlaneCount = mDimension*(mDimension - 1) / 2;
		for (int i = 0; i < PlaneCount; i++)
			delete[] mRotationPlanes[i];
		delete[] mRotationPlanes;
	}
}

int optimizercore::OptimizerMultiMap::GetNumberOfMaps() const
{
	return mNumberOfMaps;
}

void optimizercore::OptimizerMultiMap::GetImage(double x, double y[])
{
	switch (mMapType)
	{
	case MultimapType::Set:
	{
		double del;
		int i, intx = (int)floor(x);
		x = x - intx;

		if (intx == 0)
			del = 0.0;
		else
			for (i = 1, del = 1; i < intx + 1; del /= 2, i++);

		mapd(x, mTightness, y, mDimension);

		for (i = 0; i < mDimension; i++)
			y[i] = 2 * y[i] + 0.5 - del;
	}
	break;
	case MultimapType::Rotated:
	{
		int intx = (int)floor(x);//����� ���������
		x = x - intx;//������� ����� x
		mapd(x, mTightness, y, mDimension);//�������� ����� y[] � �������� �����������
		if (intx == 0 || mNumberOfMaps == 1)
			return;//���� ��������� �������� ��� ���� ��������� - ����� ������ �� ������
		int PlaneCount = mDimension*(mDimension - 1) / 2;
		int PlaneIndex = intx - 1;//������ PlaneNumber - ����� ������������
		PlaneIndex = PlaneIndex % PlaneCount;
		//�������������� ���������
		double tmp = y[mRotationPlanes[PlaneIndex][1]];
		y[mRotationPlanes[PlaneIndex][1]] = y[mRotationPlanes[PlaneIndex][0]];
		y[mRotationPlanes[PlaneIndex][0]] = -tmp;

		if (intx > PlaneCount)//������ ���� ��������������
		{
			y[mRotationPlanes[PlaneIndex][0]] = -y[mRotationPlanes[PlaneIndex][0]];
			y[mRotationPlanes[PlaneIndex][1]] = -y[mRotationPlanes[PlaneIndex][1]];
		}
	}
	break;
	case MultimapType::Noninjective:
		mapd(x, mTightness, y, mDimension, 3);
		break;
	}
}

int optimizercore::OptimizerMultiMap::GetAllPreimages(double * p, double xp[])
{
	switch (mMapType)
	{
	case MultimapType::Set:
	{
		int i, j;
		double xx;
		double del;
		del = 0.5;
		for (i = 1; i < mNumberOfMaps; i++)
		{
			for (j = 0; j < mDimension; j++)
				p2[j] = (p[j] + del - 0.5) * 0.5;

			xyd(&xx, mTightness, p2, mDimension);
			xp[i] = xx + i;
			del *= 0.5;
		}
		del = 0.0;
		for (j = 0; j < mDimension; j++)
			p2[j] = (p[j] + del - 0.5) * 0.5;
		xyd(&xx, mTightness, p2, mDimension);
		xp[0] = xx;
	}
	break;
	case MultimapType::Rotated:
	{
		int i, j;
		double xx;
		for (j = 0; j < mDimension; j++) p2[j] = p[j];

		xyd(&xx, mTightness, p2, mDimension);
		xp[0] = xx;
		//���� ���� ��������� - ����� ������ �� ������
		if (mNumberOfMaps == 1)return 1;

		int PlaneCount = mDimension*(mDimension - 1) / 2;//����� ����������

		for (i = 1; i < mNumberOfMaps; i++)
		{
			for (j = 0; j < mDimension; j++)
				p2[j] = p[j];
			//�������� �������������� ���������
			int PlaneIndex = (i - 1) % PlaneCount;

			double tmp = p[mRotationPlanes[PlaneIndex][1]];
			p2[mRotationPlanes[PlaneIndex][1]] = -p[mRotationPlanes[PlaneIndex][0]];
			p2[mRotationPlanes[PlaneIndex][0]] = tmp;

			if (i > PlaneCount)//������ ���� ��������������
			{
				p2[mRotationPlanes[PlaneIndex][0]] = -p2[mRotationPlanes[PlaneIndex][0]];
				p2[mRotationPlanes[PlaneIndex][1]] = -p2[mRotationPlanes[PlaneIndex][1]];
			}

			xyd(&xx, mTightness, p2, mDimension);
			xp[i] = xx + (i);
		}
	}
	break;
	case MultimapType::Noninjective:
	{
		int preimNumber;
		invmad(mTightness, xp, MAX_PREIMAGES, &preimNumber, p, mDimension, 4);
		return preimNumber;
	}
	break;
	}
	return mNumberOfMaps;
}

void optimizercore::OptimizerMultiMap::InitRotatedMap()
{
	p2 = new double[mDimension];
	mTmpVector = SharedVector(p2, utils::array_deleter<double>());

	int PlaneCount = mDimension*(mDimension - 1) / 2;//����� ����������
	int** mRotationPlanes = new int*[PlaneCount];//������ ���� ����������, ������ ������� ����� ����������� ��������
	for (int i = 0; i < PlaneCount; i++)
		mRotationPlanes[i] = new int[2];

	const int k = 2;//������������ �� ���� ���������
	int plane[k];//��� ������ ��� ��������

	for (int i = 0; i < k; i++)
		plane[i] = i;

	if (mDimension <= k)
	{
		for (int i = 0; i < k; i++)
			mRotationPlanes[0][i] = plane[i];
		return;
	}
	int p = k - 1;
	int counter = 0;//������� ����� ������������
	while (p >= 0)
	{
		for (int i = 0; i < k; i++)
			mRotationPlanes[counter][i] = plane[i];
		counter++;

		if (plane[k - 1] == mDimension - 1)
			p--;
		else
			p = k - 1;

		if (p >= 0)
			for (int i = k - 1; i >= p; i--)
				plane[i] = plane[p] + i - p + 1;
	}
}
