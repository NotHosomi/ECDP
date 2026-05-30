#pragma once
#include <vector>
#include <string>
#include "CvData.h"
#include "CilData.h"

struct T_DeviceData
{
	std::map<std::string, std::vector<double>> EisKeyvals;
	std::vector<T_CvData> CvData;
	T_Stats CvStats = {};
	T_CilData Cils = {};
};

