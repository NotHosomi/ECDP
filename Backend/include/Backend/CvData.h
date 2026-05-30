#pragma once
#include <vector>
#include <nlohmann\json.hpp>
#include "stddev.h"

struct T_CvLoop
{
	int nLoopIndex = -1;
	std::vector<double> vCurrents;
	std::vector<double> vVoltages;
	NLOHMANN_DEFINE_TYPE_INTRUSIVE(T_CvLoop, nLoopIndex, vCurrents, vVoltages);
};

struct T_CvElectrodeData
{
	std::vector<T_CvLoop> vLoops;
	double dCsc = -1.0;
	double dCscNorm = -1.0;
	NLOHMANN_DEFINE_TYPE_INTRUSIVE(T_CvElectrodeData, vLoops, dCsc, dCscNorm);
};

struct T_CvData
{
	std::map<std::string, T_CvElectrodeData> mElectrodes;
	T_Stats tCsc;
	T_Stats tCscNorm;
	NLOHMANN_DEFINE_TYPE_INTRUSIVE(T_CvData, mElectrodes, tCsc, tCscNorm);
};