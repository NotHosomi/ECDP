#pragma once
#include <string>
#include <filesystem>
#include "ErrorBarData.h"
#include "CvData.h"

struct T_CilData;

class Grapher
{
public:
	Grapher(std::filesystem::path outputDir);

	void GraphEIS(std::string sId, T_ErrorBarD tZ, T_ErrorBarD tPhase);
	void GraphCV(std::string sId, T_ErrorBarD tLoop);
	void GraphCV(std::string sId, std::string filename, T_CvData tLoop);
	void GraphCIL(std::string sId, const T_CilData& data);

	void GraphRawCV(std::string Id); // create a plot for each electrode
private:
	std::filesystem::path m_PlotDir;
};

