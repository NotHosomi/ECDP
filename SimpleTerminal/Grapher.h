#pragma once
#include <string>
#include <filesystem>
#include "ErrorBarData.h"

class Grapher
{
public:
	Grapher(std::filesystem::path outputDir);

	void GraphEIS(std::string sId, T_ErrorBarF tZ, T_ErrorBarF tPhase);
	void GraphCV(std::string path, std::string Id);
	void GraphCIL(std::string path, std::string Id);

private:
	std::filesystem::path m_PlotDir;
};

