#pragma once
#include <string>
#include <vector>
#include <map>
#include <nlohmann\json.hpp>

struct T_EisRawData
{
	std::vector<double> vFrequencies;
	std::vector<double> vImpedances;
	std::vector<double> vPhases;
	NLOHMANN_DEFINE_TYPE_INTRUSIVE(T_EisRawData, vFrequencies, vImpedances, vPhases);
};

struct T_EisData
{
	std::vector<std::string> vFrequencies;
	std::vector<double> vAverages; 
	std::vector<double> vStddev;
	std::map<std::string, std::vector<double>> mImpedances;

	std::map<std::string, T_EisRawData> mRaw;
	NLOHMANN_DEFINE_TYPE_INTRUSIVE(T_EisData, vFrequencies, vAverages, vStddev, mImpedances);
};