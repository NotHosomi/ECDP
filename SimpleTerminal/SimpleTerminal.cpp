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

std::string RoundToStr(double num) { return std::to_string(static_cast<int>(num + 0.5)); }

int main(int argc, char* argv[])
{
	std::string dataPath = "";
	std::string deviceId = "";
	bool bPlotAll = false;
	for (int i = 1; i < argc; ++i)
	{
		if (strcmp(argv[i], "-p") == 0 && i < argc - 1)
		{
			if (std::filesystem::exists(argv[i + 1]))
			{
				dataPath = argv[i + 1];
				std::cout << "Data directory: " << std::filesystem::path(dataPath) << std::endl;
				i += 1;
			}
			else
			{
				std::cout << "Could not find data directory \"" << argv[i + 1] << "\"";
			}
		}
		else if (strcmp(argv[i], "-d") == 0 && i < argc - 1)
		{
			deviceId = argv[i + 1];
			std::cout << "deviceId: " << deviceId << std::endl;
			i += 1;
		}
		else if (strcmp(argv[i], "--PlotAll") == 0)
		{
			bPlotAll = true;
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
	while (!std::filesystem::exists(dataPath))
	{
		std::cout << "Input data path: ";
		std::cin >> dataPath;
		std::cin.clear();
	}

	std::string devicePath = "";
	Grapher grapher(dataPath);
	for(;;)
	{
		if (deviceId == "")
		{
			std::cout << "Input device ID: ";
			std::cin >> deviceId;
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

		// Load EIS
		std::cout << "\nFetching EIS values..." << std::endl;
		std::map<std::string, std::array<double,3>> ImpedanceKeyvals = ingest.GetEisKeyvals();

		// EIS Table
		PrintTable EisTable({ "Electrode", "100 Hz", "1000 Hz", "1995 Hz" });
		double sum = 0.0;
		int validCount = 0;
		for (const auto& iter : ImpedanceKeyvals)
		{
			std::string colour = TERM_RED;
			if (iter.second[1] <= 20000)
			{
				sum += iter.second[0];
				validCount += 1;
				colour = TERM_GREEN;
			}
			EisTable.AddRow({iter.first, RoundToStr(iter.second[0]), RoundToStr(iter.second[1]), RoundToStr(iter.second[2]) }, colour);
		}
		EisTable.Print(TERM_BOLDRED);
		std::cout << "  Average: " << (sum / validCount) << std::endl;

		// EIS Plot
		std::array<T_ErrorBarD, 2> EisData = ingest.GetEisPlot();
		grapher.GraphEIS(deviceId, EisData[0], EisData[1]);

		// Load CV
		std::map<std::string, T_CvData> mCv = ingest.CalculateCscVals();
		PrintTable CscTable({ "Electrode", "CSC (mC/cm^2)" });
		sum = 0.0;
		for (const auto& iter : mCv)
		{
			CscTable.AddRow({ iter.first, std::to_string(iter.second.dCsc) });
			sum += iter.second.dCsc;
		}
		CscTable.Print(TERM_BOLDBLUE);
		std::cout << "  Average: " << (sum / mCv.size()) << std::endl;

		// CV Plot
		T_ErrorBarD tCvPlot = ingest.GetCvPlot();
		grapher.GraphCV(deviceId, tCvPlot);
		if (bPlotAll)
		{
			for (const auto& [key, data] : mCv)
			{
				grapher.GraphCV(deviceId, key, data);
			}
		}

		// CIL
		T_CilData cils = ingest.CalculateCilVals();
		std::vector<std::string> cilTableHeaders{ "Electrode #" };
		for (const auto& pulseWidth : cils.vPulseWidths) { cilTableHeaders.push_back(std::to_string(pulseWidth) + "us"); };
		PrintTable cilTable(cilTableHeaders);
		for (const auto& row : cils.mCilVals)
		{
			std::vector<std::string> rowtext{ std::to_string(row.first)};
			for (const auto& val : row.second) { rowtext.push_back(std::to_string(val)); };
			cilTable.AddRow(rowtext);
		}
		cilTable.Print(TERM_YELLOW);
		std::cout << "  Average: " << (sum / mCv.size()) << std::endl;


		// Plot CIL values
		grapher.GraphCIL(deviceId, cils);
		//todo


		deviceId = "";
	}
}