// SimpleTerminal.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <filesystem>
#include <array>
#include <fstream>
#include "TerminalColours.h"
#include "Ingester.h"
#include "ErrorBarData.h"
#include "Grapher.h"
#include "PrintTable.h"
#include "UserConfig.h"
#include "JsonLoader.h"

std::string RoundToStr(double num) { return std::to_string(static_cast<int>(num + 0.5)); }

int main(int argc, char* argv[])
{
	std::string deviceId = "";

	T_UserConfig tUserConfig;
	std::cout << "Loading user config..." << std::flush;
	if (LoadJson<T_UserConfig>("./UserConfig.json", tUserConfig))
	{
		std::cout << "Failed -- Creating default..." << std::flush;
		SaveJson("./UserConfig.json", tUserConfig);
	}
	std::cout << "Done" << std::endl;

	for (int i = 1; i < argc; ++i)
	{
		if (strcmp(argv[i], "--data") == 0 && i < argc - 1)
		{
			if (std::filesystem::exists(argv[i + 1]))
			{
				std::cout << "Overriding data directory (cmdopt)" << std::endl;
				tUserConfig.dataDirectory = argv[i + 1];
				i += 1;
			}
			else
			{
				std::cout << "Could not find data directory \"" << argv[i + 1] << "\"";
			}
		}
		else if (strcmp(argv[i], "--dev") == 0 && i < argc - 1)
		{
			deviceId = argv[i + 1];
			std::cout << "deviceId: " << deviceId << std::endl;
			i += 1;
		}
		else if (strcmp(argv[i], "--each") == 0 && i < argc - 1)
		{
			tUserConfig.eis.plotEachElectrode = argv[i + 1][0] == '0';
			tUserConfig.cv.plotEachElectrode = argv[i + 1][0] == '0';
			tUserConfig.cil.plotEachElectrode = argv[i + 1][0] == '0';
			i += 1;
		}
		else if (strcmp(argv[i], "--plotEis") == 0 && i < argc - 1)
		{
			tUserConfig.eis.plotEis = argv[i + 1][0] == '0';
			i += 1;
		}
		else if (strcmp(argv[i], "--calcCsc") == 0 && i < argc - 1)
		{
			tUserConfig.cv.calcCsc = argv[i + 1][0] == '0';
			i += 1;
		}
		else if (strcmp(argv[i], "--plotCv") == 0 && i < argc - 1)
		{
			tUserConfig.cv.plotCv = argv[i + 1][0] == '0';
			i += 1;
		}
		else if (strcmp(argv[i], "--plotCil") == 0 && i < argc - 1)
		{
			tUserConfig.cv.plotCv = argv[i + 1][0] == '0';
			i += 1;
		}
		else
		{
			std::cout << "unrecognised parameter \"" << argv[i] << "\"" << std::endl;
			std::cout << "aborting" << std::endl;
			return 1;
		}
	}

	//SetConsoleOutputCP(CP_UTF8);
	std::cout << TERM_RESET;
	//"C:\\Users\\Hosomi\\OneDrive - Imperial College London\\Data\\";
	std::string dataPath = tUserConfig.dataDirectory;
	if (!std::filesystem::exists(dataPath))
	{
		do
		{
			std::cout << "Input data path: ";
			std::cin >> dataPath;
			std::cin.clear();
		} while (!std::filesystem::exists(dataPath));
	}
	else
	{

		std::cout << "Data directory: " << std::filesystem::path(dataPath) << std::endl;
	}

	std::string devicePath = "";
	Grapher grapher(dataPath);
	for(;;)
	{
		if (deviceId == "")
		{
			std::cout << "Input device ID: ";
			std::cin >> deviceId;
			if (deviceId == "quit")
			{
				break;
			}
			std::cin.clear();
		}
		devicePath = dataPath + "/" + deviceId;
		if (!std::filesystem::exists(devicePath))
		{
			std::cout << "Could not find " << deviceId << std::endl;
			deviceId = "";
			continue;
		}

		Ingester ingest(devicePath);

		// EIS
		if (tUserConfig.eis.fetchKeyvals)
		{
			std::cout << "\nFetching EIS values..." << std::endl;
			std::map<std::string, std::vector<double>> ImpedanceKeyvals = ingest.GetEisKeyvals(tUserConfig.eis.keyVals);

			// EIS Table
			std::vector<std::string> headers;
			headers.push_back("Electrode");
			for (auto keyval : tUserConfig.eis.keyVals)
			{
				headers.push_back(keyval + " Hz");
			}
			PrintTable EisTable(headers);
			std::vector<std::string> eisExclusionList;
			int validCount = 0;
			for (const auto& iter : ImpedanceKeyvals)
			{
				std::string colour = TERM_RED;
				if (iter.second[1] <= tUserConfig.eis.maxValidImpedance)
				{
					validCount += 1;
					colour = TERM_GREEN;
				}
				EisTable.AddRow({ iter.first, RoundToStr(iter.second[0]), RoundToStr(iter.second[1]), RoundToStr(iter.second[2]) }, colour);
			}
			EisTable.Print(TERM_BOLDRED);

			// EIS Plot
			if (tUserConfig.eis.plotEis)
			{
				std::array<T_ErrorBarD, 2> EisData = ingest.GetEisPlot();
				grapher.GraphEIS(deviceId, EisData[0], EisData[1]);
			}
			if (tUserConfig.eis.plotEis)
			{
				// todo: add this to the grapher
			}
		}

		// CV
		std::vector<std::string> cvExcludes;
		if (tUserConfig.cv.calcCsc)
		{
			std::map<std::string, T_CvData> mCv = ingest.CalculateCscVals();
			PrintTable CscTable({ "Electrode", "CSC (mC/cm^2)" });
			double sum = 0.0;
			for (const auto& iter : mCv)
			{
				std::string colour = "";
				if (iter.second.dCsc < 1 || iter.second.dCsc != iter.second.dCsc)
				{
					colour = TERM_RED;
					cvExcludes.push_back(iter.first);
				}
				CscTable.AddRow({ iter.first, std::to_string(iter.second.dCsc) }, colour);
				sum += iter.second.dCsc;
			}
			CscTable.Print(TERM_BOLDBLUE);
			std::cout << "  Average: " << (sum / mCv.size()) << std::endl;

			if (tUserConfig.cv.plotEachElectrode)
			{
				for (const auto& [key, data] : mCv)
				{
					grapher.GraphCV(deviceId, key, data);
				}
			}
		}
		// CV Plot
		if (tUserConfig.cv.plotCv)
		{
			T_ErrorBarD tCvPlot = ingest.GetCvPlot(cvExcludes);
			grapher.GraphCV(deviceId, tCvPlot);
		}

		// CIL
		if(tUserConfig.cil.calcCil)
		{
			T_CilData cils = ingest.CalculateCilVals();
			std::vector<std::string> cilTableHeaders{ "Electrode #" };
			for (const auto& pulseWidth : cils.vPulseWidths) { cilTableHeaders.push_back(std::to_string(pulseWidth) + "us"); };
			PrintTable cilTable(cilTableHeaders);
			for (const auto& row : cils.mCilVals)
			{
				std::vector<std::string> rowtext{ std::to_string(row.first) };
				for (const auto& val : row.second) { rowtext.push_back(std::to_string(val)); };
				cilTable.AddRow(rowtext);
			}
			std::vector<std::string> rowtext{ "{AVRG}" };
			for (const auto& val : cils.vAvrgCils) { rowtext.push_back(std::to_string(val)); };
			cilTable.AddRow(rowtext);

			cilTable.Print(TERM_YELLOW);
			// todo: show average CIL for each pulse width
			//std::cout << "  Average: " << (sum / mCv.size()) << std::endl;

			if (tUserConfig.cil.plotCil)
			{
				// Plot CIL values
				grapher.GraphCIL(deviceId, cils);
			}
			if (tUserConfig.cil.plotEachElectrode)
			{

			}
		}

		std::cout << "\nFinished " << deviceId << "\n" << std::endl;
		deviceId = "";
	}
}