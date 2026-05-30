#pragma once
#include <memory>
#include "dllspec.h"
#include "GrapherGeneric.h"
#include "stddev.h"
#include "Archive.h"
#include "EisData.h"
#include "CvData.h"
#include "CilData.h"

// core will need overhauling?

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
	DLL Core();

	enum E_DataTypes
	{
		kNone = 0,
		kEis = 1,
		kCv = 1 << 1,
		kCil = 1 << 2,
		kAll = kEis | kCv | kCil
	};
	DLL E_CmdErr Run(const std::string sDeviceId, E_DataTypes eModes);
	DLL E_CmdErr Plot(const std::string sDeviceId, E_DataTypes eModes);
	DLL bool BatchAverages(const std::vector<std::string> sIds);


private:
	T_EisData Eis(T_DeviceData& tDeviceData, const Ingester& ingest);
	T_CvData Cv(T_DeviceData& tDeviceData, const Ingester& ingest);
	T_CilData Cil(T_DeviceData& tDeviceData, const Ingester& ingest);

	void PrintEisVals(const T_EisData& tEisData, const std::vector<std::string>& vKeyVals);
	void PrintCscVals(const T_CvData& tCvData);
	void PrintCilVals(std::vector<int> vPulseWidths, std::map<int, std::vector<float>> mVals, std::vector<T_Stats> vStats);

	void PlotEis(T_DeviceData& tDeviceData, bool bForced = false);
	void PlotCv(T_DeviceData& tDeviceData, bool bForced = false);
	void PlotCil(T_DeviceData& tDeviceData, bool bForced = false);

	std::array<T_ErrorPlotF, 2> BuildEisPlot(const T_EisData& tData);
	T_ErrorPlotF BuildCvPlot(const T_CvData& tData);
	T_ErrorPlotF BuildCilPlot(const T_CilData& tData);

	bool LoadGrapher();

	std::unique_ptr<GrapherGeneric> m_pGrapher;
	Archive m_Archive;
};

