#pragma once
#include <string>
#include <filesystem>
#include "ErrorBarData.h"
#include "CvData.h"

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

	void GraphEIS(std::string sId, T_ErrorBarD tZ, T_ErrorBarD tPhase);
	void GraphCV(std::string sId, T_ErrorBarD tLoop);
	void GraphCV(std::string sId, std::string filename, T_CvData tLoop);
	void GraphCIL(std::string sId, const T_CilData& data);

	std::string GetGraphPath(std::string sId, E_GraphType eType);

private:
	std::filesystem::path m_PlotDir;

	int m_nEisHeight = 970;
	int m_nEisWidth = 1280;
	int m_nCvHeight = 970;
	int m_nCvWidth = 1280;
	int m_nCilHeight = 970;
	int m_nCilWidth = 1280;
};

