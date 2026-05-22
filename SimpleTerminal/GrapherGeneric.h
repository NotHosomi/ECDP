#pragma once
#include <string>
#include "Plottables.h"

struct T_CvElectrodeData;
struct T_CilData;
struct T_EisRawData;

class GrapherGeneric
{
public:
	GrapherGeneric() = default;
	void SetOutputPath(const std::string& outputDir);

	virtual void EisAverage(const std::string& sId, const T_ErrorPlotF& tZ, const T_ErrorPlotF& tPhase) = 0;
	virtual void EisSingle(const std::string& sId, const std::string& filename, const T_EisRawData& tRaw) = 0;

	virtual void CvAverage(const std::string& sId, T_ErrorPlotF tLoop) = 0;
	virtual void CvSingle(const std::string& sId, const std::string& filename, T_CvElectrodeData tLoop) = 0;

	virtual void CilAverage(const std::string& sId, const T_ErrorPlotF& data) = 0;
	virtual void CilMulti(const std::string& sId, const T_CilData& data) = 0;

protected:
	std::string m_sOutputPath = "/Plots/";
};

