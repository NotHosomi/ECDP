#pragma once
#include "UserConfig.h"
#include "Grapher.h"
#include "stddev.h"

class Ingester;

enum E_CmdErr
{
	None = 0,
	BadCmd,
	NotEnoughArgs,
	TooManyArgs,
	InvalidArgTypes,
	BadArgs,
	NoData,
	Other
};

class Core
{
public:
	Core();


	void Eis(const std::string sDeviceId, const Ingester& ingest, const T_EisConfig& tUserConfig);
	void Cv(const std::string sDeviceId, const Ingester& ingest, const T_CvConfig& tUserConfig);
	void Cil(const std::string sDeviceId, const Ingester& ingest, const T_CilConfig& tUserConfig);

	T_UserConfig& UserConfig();

private:
	void PrintCilVals(std::vector<int> vPulseWidths, std::map<int, std::vector<float>> mVals, std::vector<T_Stats> vStats);

	std::filesystem::path m_DataPath;
	Grapher m_Grapher;
	T_UserConfig m_tUserConfig;
};

