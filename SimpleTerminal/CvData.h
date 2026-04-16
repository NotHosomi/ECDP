#pragma once
#include <vector>

struct T_CvLoop
{
	int nLoopIndex;
	std::vector<double> vCurrents;
	std::vector<double> vVoltages;
};

struct T_CvData
{
	std::vector<T_CvLoop> vLoops;
	double dCsc;
};