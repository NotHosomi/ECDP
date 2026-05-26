#pragma once
#include <memory>
#include "UserConfig.h"
#include "GrapherGeneric.h"
#include "stddev.h"
#include "Archive.h"
#include "EisData.h"
#include "CvData.h"
#include "CilData.h"

class Ingester;
struct T_CvData;

enum E_CmdErr
{
	None = 0,
	BadCmd,
	NotEnoughArgs,
	TooManyArgs,
	InvalidArgTypes,
	BadArgs,
	NoData,
	BadOptions,
	Other
};

class Core
{
public:
	Core();

	enum E_DataTypes
	{
		kNone = 0,
		kEis = 1,
		kCv = 1 << 1,
		kCil = 1 << 2
	};
	E_CmdErr Run(const std::string sDeviceId, E_DataTypes eModes);
	bool BatchAverages(const std::vector<std::string> sIds);

	T_UserConfig& UserConfig();

private:
	T_EisData Eis(T_DeviceData& tDeviceData, const Ingester& ingest, const T_EisConfig& tUserConfig);
	T_CvData Cv(T_DeviceData& tDeviceData, const Ingester& ingest, const T_CvConfig& tUserConfig);
	T_CilData Cil(T_DeviceData& tDeviceData, const Ingester& ingest, const T_CilConfig& tUserConfig);

	void PrintEisVals(const T_EisData& tEisData, const T_EisConfig& tConfig);
	void PrintCscVals(const T_CvData& tCvData);
	void PrintCilVals(std::vector<int> vPulseWidths, std::map<int, std::vector<float>> mVals, std::vector<T_Stats> vStats);

	bool LoadGrapher();

	std::filesystem::path m_DataPath;
	std::unique_ptr<GrapherGeneric> m_pGrapher;
	Archive m_Archive;
	T_UserConfig m_tUserConfig;
};

