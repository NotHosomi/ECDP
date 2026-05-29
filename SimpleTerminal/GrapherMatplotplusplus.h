#pragma once
#include <string>
#include <filesystem>
#include "GrapherGeneric.h"
#include "Plottables.h"
#include "CvData.h"


class GrapherMatplotplusplus : public GrapherGeneric
{
public:
	void EisAverage(const std::string& sId, const T_ErrorPlotF& tZ, const T_ErrorPlotF& tPhase, bool bReplot = false) override;
	void EisSingle(const std::string& sId, const std::string& filename, const T_EisRawData& tRaw, bool bReplot = false) override;

	void CvAverage(const std::string& sId, T_ErrorPlotF tLoop, bool bReplot = false) override;
	void CvSingle(const std::string& sId, const std::string& filename, T_CvElectrodeData tLoop, bool bReplot = false) override;

	void CilAverage(const std::string& sId, const T_ErrorPlotF& data, bool bReplot = false) override;
	void CilMulti(const std::string& sId, const T_CilData& data, bool bReplot = false) override;
private:
	int m_nEisHeight = 970;
	int m_nEisWidth = 1280;
	int m_nCvHeight = 970;
	int m_nCvWidth = 1280;
	int m_nCilHeight = 970;
	int m_nCilWidth = 1280;
};

