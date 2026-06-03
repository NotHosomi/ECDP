#pragma once
#include <map>
#include <vector>

using CvLoop = std::map<float, float>;

struct T_CvLoop
{
	float fArea;
	CvLoop mLoop;
};
struct T_CvElectrode
{
	int nElectrodeId;
	float fAverageArea;
	float fStddev;
	std::vector<T_CvLoop> vData;
};
struct T_CvGross
{
	float fAverage;
	float fStddev;
	std::vector<T_CvElectrode> vData;
};
