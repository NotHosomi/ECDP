#pragma once
#include <string>
#include <filesystem>
#include "ErrorBarData.h"

class Ingester;
struct T_CvElectrodeData;
struct T_CilData;

enum class E_GraphType
{
	Eis,
	Cv,
	Cil
};


class Grapher
{
public:
	Grapher(std::filesystem::path outputDir = "./plots/");
	void SetOutputPath(std::filesystem::path outputDir);

	void GraphDeviceEIS(const std::string& sId, const T_ErrorBarD& tZ, const T_ErrorBarD& tPhase);
	void GraphElectrodeEIS(const std::string& sId, const Ingester& ingest);
	void GraphDeviceCV(const std::string& sId, T_ErrorBarD tLoop);
	void GraphElectrodeCV(const std::string& sId, const std::string& filename, T_CvElectrodeData tLoop);
	void GraphDeviceCIL(const std::string& sId, const T_CilData& data);

	std::string GetGraphPath(const std::string& sId, E_GraphType eType);

private:
	std::filesystem::path m_PlotDir;

	int m_nEisHeight = 970;
	int m_nEisWidth = 1280;
	int m_nCvHeight = 970;
	int m_nCvWidth = 1280;
	int m_nCilHeight = 970;
	int m_nCilWidth = 1280;
};

