#include "Ingester.h"
#include <filesystem>
#include <iostream>
#include <fstream>
#include <assert.h>
#include <map>
#include "TerminalColours.h"
#include "CsvFile.h"
#include "stddev.h"

# define M_PI           3.14159265358979323846

Ingester::Ingester(std::filesystem::path deviceDirectory)
{
	std::cout << "Files found..." << std::endl;
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


	if (!std::filesystem::exists(deviceDirectory / "Details.txt"))
	{
		std::cout << "Device details not found." << std::endl;
		std::cout << "Electrode diameter (microns): ";
		while (!(std::cin >> m_fElectrodeDiameter))
		{
			std::cin.clear();
			std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
		}
		std::ofstream file(deviceDirectory / "details.txt");
		file << "diameter:" << m_fElectrodeDiameter << std::endl;
		file.close();
	}
	else
	{
		// todo: expand with more details if necessary
		std::ifstream file(deviceDirectory / "details.txt");
		std::string str;
		std::getline(file, str);
		file.close();
		m_fElectrodeDiameter = static_cast<float>(std::atof(str.substr(str.find(":") + 1).c_str()));
		if(m_fElectrodeDiameter == 0)
		{
			std::cout << "Failed to read details.txt on line \"" + str + "\"" << std::endl;
			std::cout << "Some things may not work correctly" << std::endl;
		}
	}
}

std::vector<CsvFile> Ingester::readFiles(const std::vector<std::filesystem::path>& fileaddrs)
{
	std::vector<CsvFile> csvList;
	for (auto& path : fileaddrs)
	{
		csvList.emplace_back(path.string(), ';');
		if (csvList.back().GetHeadings().size() == 1) // in case it has a different delimiter
		{
			csvList.back() = CsvFile(path.string(), ',');
		}
	}
	return csvList;
}

double Ingester::hysteresisArea(const std::vector<double>& x, const std::vector<double>& y)
{
	double area = 0.0;
	int n = static_cast<int>(x.size());
	int j;
	for (int i = 0; i < n; ++i)
	{
		j = (i + 1) % n;
		area += x[i] * y[j] - x[j] * y[i];
	}
	return std::abs(area) / 2.0;
}

std::array<T_ErrorBarD, 2> Ingester::GetEisPlot()
{
	std::cout << "\nGenerating EIS plot..." << std::flush;
	std::vector<CsvFile> csvList = readFiles(m_vEisPaths);

	std::erase_if(csvList, [](const CsvFile& data) {
		for (int i = 0; i < data.GetCol(0).size(); ++i)
		{
			if (data.GetCol("Frequency (Hz)")[i] == "1000")
			{
				return std::atof(data.GetCol("Z (\xCE\xA9)")[i].c_str()) >= 20000;
			}
		}
		return true;
		});
	if (csvList.size() == 0) { return { T_ErrorBarD(), T_ErrorBarD() }; }

	T_ErrorBarD PointsZ;
	T_ErrorBarD PointsPhase;
	for (const auto& freq : csvList[0].GetCol("Frequency (Hz)"))
	{
		PointsZ.x.push_back(std::stof(freq));
		PointsPhase.x.push_back(-std::stof(freq));
	}

	for (int rowindex = 0; rowindex < csvList[0].GetCol(0).size(); ++rowindex)
	{
		float avrgZY = 0;
		std::vector<double> rowZVals;
		std::vector<double> rowPhaseVals;
		float avrgPhaseY = 0;
		for (int fileindex = 0; fileindex < csvList.size(); ++fileindex)
		{
			rowZVals.push_back(std::stof(csvList[fileindex].GetCol("Z (\xCE\xA9)")[rowindex]));
			rowPhaseVals.push_back(-std::stof(csvList[fileindex].GetCol("-Phase (\xC2\xB0)")[rowindex]));
		}
		T_Stats rowZStats = stddev(rowZVals);
		PointsZ.y.push_back(rowZStats.mean);
		PointsZ.err.push_back(rowZStats.stddev);
		T_Stats rowPhaseStats = stddev(rowPhaseVals);
		PointsPhase.y.push_back(rowPhaseStats.mean);
		PointsPhase.err.push_back(rowPhaseStats.stddev);
	}
	std::cout << " Done" << std::endl;
	return { PointsZ, PointsPhase };
}

std::map<std::string, std::array<double, 3>> Ingester::GetEisKeyvals()
{
	std::map<std::string, std::array<double, 3>> out;
	std::vector<CsvFile> csvList = readFiles(m_vEisPaths);
	for (const auto& entry : csvList)
	{
		std::array<double, 3> keyvals{ 100000.0, 100000.0, 100000.0 };
		int index = 0;
		for (; index < entry.GetCol(0).size(); ++index)
		{
			if (entry.GetCol("Frequency (Hz)")[index] == "100")
			{
				keyvals[0] = std::atof(entry.GetCol("Z (\xCE\xA9)")[index].c_str());
			}
			else if (entry.GetCol("Frequency (Hz)")[index] == "1000")
			{
				keyvals[1] = std::atof(entry.GetCol("Z (\xCE\xA9)")[index].c_str());
			}
			else if (entry.GetCol("Frequency (Hz)")[index] == "1995.3")
			{
				keyvals[2] = std::atof(entry.GetCol("Z (\xCE\xA9)")[index].c_str());
			}
		}
		out.insert({ entry.GetFilename(), keyvals });
	}
	return out;
}

std::map<std::string, double> Ingester::CalculateCscVals()
{
	std::map<std::string, double> mCscVals;
	std::cout << "Reading CV..." << std::endl;
	std::vector<CsvFile> csvList = readFiles(m_vCvPaths);

	std::cout << "Calculating CSCs..." << std::endl;
	for(auto entry : csvList)
	{
		std::vector<std::string> scanStrs = entry.GetCol("Scan");
		std::vector<std::string> voltageStrs = entry.GetCol("WE(1).Potential (V)");
		std::vector<std::string> currentStrs = entry.GetCol("WE(1).Current (A)");

		std::vector<int> vLoopOffsets;
		std::vector<std::vector<double>> currents;
		std::vector<std::vector<double>> voltages;
		vLoopOffsets.push_back(0);
		currents.emplace_back();
		voltages.emplace_back();
		for (int i = 0; i < entry.GetCol(0).size(); ++i)
		{
			int loop = std::atoi(scanStrs[i].c_str());
			if (loop != vLoopOffsets.size())
			{
				vLoopOffsets.push_back(i);
				currents.emplace_back();
				voltages.emplace_back();
			}
			currents[loop - 1].push_back(std::atof(currentStrs[i].c_str()));
			voltages[loop - 1].push_back(std::atof(voltageStrs[i].c_str()));
		}

		double area = 0;
		for (int i = 1; i < voltages.size(); ++i)
		{
			area += hysteresisArea(voltages[i], currents[i]);
		}
		area /= voltages.size();

		// todo: convert area to CSC
		double timeDelta = std::atof(entry.GetCol("Time (s)")[vLoopOffsets[2]/2].c_str())
						 - std::atof(entry.GetCol("Time (s)")[vLoopOffsets[1]].c_str());
		double voltDelta = std::atof(entry.GetCol("WE(1).Potential (V)")[vLoopOffsets[2]/2].c_str())
						 - std::atof(entry.GetCol("WE(1).Potential (V)")[vLoopOffsets[1]].c_str());
		double scanRate = std::abs(voltDelta / timeDelta);
		double csc = (area / (2 * scanRate * GetElectrodeArea_cm2())) * 1000; // C -> mC
		mCscVals.insert({ entry.GetFilename(), csc });
	}
	return mCscVals;
}

float Ingester::GetElectrodeDiameter()
{
	return m_fElectrodeDiameter;
}

double Ingester::GetElectrodeArea_cm2()
{
	return GetElectrodeArea_um2() / 1e8;
}

double Ingester::GetElectrodeArea_um2()
{
	return std::pow(m_fElectrodeDiameter / 2, 2) * M_PI;
}
