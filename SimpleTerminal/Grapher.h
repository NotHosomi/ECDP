#pragma once
#include <string>
#include <filesystem>
#include "ErrorBarData.h"
#include "CvData.h"

class Grapher
{
public:
	Grapher(std::filesystem::path outputDir);

	void GraphEIS(std::string sId, T_ErrorBarD tZ, T_ErrorBarD tPhase);
	void GraphCV(std::string path, std::string Id, T_CvData tData);
	void GraphCIL(std::string path, std::string Id);

private:
	std::filesystem::path m_PlotDir;
};

