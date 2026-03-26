#include "Ingester.h"
#include <filesystem>
#include <iostream>
#include <assert.h>
#include "TerminalColours.h"
#include "CsvFile.h"
#include "stddev.h"

Ingester::Ingester(std::filesystem::path deviceDirectory)
{
	std::string path;
	std::string colcmd;
	std::string printPath;
	for (const auto& entry : std::filesystem::recursive_directory_iterator(deviceDirectory))
	{
		if (entry.is_regular_file())
		{
			path = entry.path().string();
			colcmd = TERM_RESET;
			if (path.find("exclude") != std::string::npos || path.find(".txt") == std::string::npos)
			{
				colcmd = TERM_MAGENTA;
			}
			else if (path.find("EIS") != std::string::npos)
			{
				colcmd = TERM_BOLDRED;
				m_vEisPaths.push_back(path);
			}
			else if (path.find("CV") != std::string::npos)
			{
				colcmd = TERM_BOLDBLUE;
				m_vCvPaths.push_back(path);
			}
			else if (path.find("CIL") != std::string::npos)
			{
				colcmd = TERM_BOLDYELLOW;
				m_vCilPaths.push_back(path);
			}
			else
			{
				colcmd = TERM_BOLDWHITE;
			}
			path.erase(0, deviceDirectory.string().length());
			std::cout << TERM_RESET << " - " << colcmd << path << std::endl;
		}
		std::cout << TERM_RESET;
	}
}

std::array<T_ErrorBarF, 2> Ingester::ReadEISFiles()
{
	std::vector<CsvFile> csvList;
	for (auto& path : m_vEisPaths)
	{
		csvList.emplace_back(path.string(), ';');
		if (csvList.back().GetHeadings().size() == 1) // in case it has a different delimiter
		{
			csvList.back() = CsvFile(path.string(), ',');
		}
	}

	T_ErrorBarF PointsZ;
	T_ErrorBarF PointsPhase;
	for (const auto& cell : csvList[0].GetCol("Frequency (Hz)"))
	{
		PointsZ.x.push_back(std::stof(cell));
		PointsPhase.x.push_back(std::stof(cell));
	}

	for (int rowindex = 0; rowindex < csvList[0].GetCol(0).size(); ++rowindex)
	{
		float avrgZY = 0;
		std::vector<float> rowZVals;
		std::vector<float> rowPhaseVals;
		float avrgPhaseY = 0;
		for (int fileindex = 0; fileindex < csvList.size(); ++fileindex)
		{
			rowZVals.push_back(std::stof(csvList[fileindex].GetCol("Z (\xCE\xA9)")[rowindex]));
			rowPhaseVals.push_back(std::stof(csvList[fileindex].GetCol("-Phase (\xC2\xB0)")[rowindex]));
		}
		T_Stats rowZStats = stddev(rowZVals);
		PointsZ.y.push_back(rowZStats.mean);
		PointsZ.err.push_back(rowZStats.stddev);
		T_Stats rowPhaseStats = stddev(rowPhaseVals);
		PointsPhase.y.push_back(rowPhaseStats.mean);
		PointsPhase.err.push_back(rowPhaseStats.stddev);
	}
	return { PointsZ, PointsPhase };
}
