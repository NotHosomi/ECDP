#include <vector>
#include <map>
#include <nlohmann\json.hpp>
#include "stddev.h"

using ElectrodeNum = int;
using Millis = int;

struct T_CilData
{
	std::vector<Millis> vPulseWidths;
	std::map<ElectrodeNum, std::vector<float>> mCilVals;
	std::map<ElectrodeNum, std::vector<float>> mCilValsNormalised;
	std::vector<T_Stats> vCilStats;
	std::vector<T_Stats> vCilStatsNormalised;
	NLOHMANN_DEFINE_TYPE_INTRUSIVE(T_CilData, vPulseWidths, mCilVals, mCilValsNormalised, vCilStats, vCilStatsNormalised);
};