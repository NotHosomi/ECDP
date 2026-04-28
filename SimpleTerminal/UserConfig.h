#pragma once
#include <string>
#include <nlohmann\json.hpp>

struct T_EisConfig
{
	float maxValidImpedance = 20000;
	std::vector<std::string> keyVals = { "100", "1000", "1995.3" };
	bool fetchKeyvals = true;
	bool plotEis = true;
	bool plotEachElectrode = false;
	NLOHMANN_DEFINE_TYPE_INTRUSIVE(T_EisConfig, maxValidImpedance, keyVals, fetchKeyvals, plotEis, plotEachElectrode);
};

struct T_CvConfig
{
	float minValidCsc = 100;
	bool calcCsc = true;
	bool plotCv = true;
	bool plotEachElectrode = false;
	NLOHMANN_DEFINE_TYPE_INTRUSIVE(T_CvConfig, minValidCsc, calcCsc, plotCv, plotEachElectrode);
};

struct T_CilConfig
{
	bool calcCil = true;
	bool plotCil = true;
	bool plotEachElectrode = false;
	NLOHMANN_DEFINE_TYPE_INTRUSIVE(T_CilConfig, calcCil, plotCil, plotEachElectrode);
};

struct T_UserConfig
{
	std::string dataDirectory;
	T_EisConfig eis;
	T_CvConfig cv;
	T_CilConfig cil;

	NLOHMANN_DEFINE_TYPE_INTRUSIVE(T_UserConfig, dataDirectory, eis, cv, cil);
};