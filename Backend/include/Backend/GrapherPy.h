#pragma once
#include <string>
#include <utility>
#include "GrapherGeneric.h"
#include "Plottables.h"

class GrapherPy : public GrapherGeneric
{
public:
	static bool Precheck();
	GrapherPy() = default;


	void EisAverage(const std::string& sName, const T_ErrorPlotF& tZ, const T_ErrorPlotF& tPhase, bool bReplot = false) override;
	void EisSingle(const std::string& sId, const std::string& filename, const T_EisRawData& tRaw, bool bReplot = false) override;

	void CvAverage(const std::string& sName, T_ErrorPlotF tLoop, bool bReplot = false) override;
	void CvSingle(const std::string& sId, const std::string& filename, T_CvElectrodeData tRaw, bool bReplot = false) override;

	void CilAverage(const std::string& sName, const T_ErrorPlotF& tCils, bool bReplot = false) override;
	void CilMulti(const std::string& sId, const T_CilData& data, bool bReplot = false) override;

private:
	std::string GetTempPlotPath(const std::string& sName, const std::string& sMode);

	void CmdEis(
		const std::string& sPlotDataPath,
		const std::string& sOutput,
		const std::string& sTitle,
		const std::pair<float, float>& tYLim,
		const std::pair<float, float>& tY2Lim,
		unsigned int uDpi,
		unsigned int uLabelFontsize,
		unsigned int uTickFontsize,
		unsigned int uTitleFontsize);


	void CmdCv(
		const std::string& sPlotDataPath,
		const std::string& sOutput,
		const std::string& sTitle,
		const std::pair<float, float>& tYLim,
		unsigned int uDpi,
		unsigned int uLabelFontsize,
		unsigned int uTickFontsize,
		unsigned int uTitleFontsize);

	void CmdCil(
		const std::string& sPlotDataPath,
		const std::string& sOutput,
		const std::string& sTitle,
		unsigned int uDpi,
		unsigned int uLabelFontsize,
		unsigned int uTickFontsize,
		unsigned int uTitleFontsize);
};

