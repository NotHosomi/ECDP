// SimpleTerminal.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <filesystem>
#include <array>
#include "TerminalColours.h"
#include "Ingester.h"
#include "ErrorBarData.h"
#include "Grapher.h"


int main()
{
	//SetConsoleOutputCP(CP_UTF8);
	std::cout << TERM_RESET;
	//"C:\\Users\\Hosomi\\OneDrive - Imperial College London\\Data\\";
	std::string dataPath = "";
	do
	{
		std::cout << "Input data path: ";
		std::cin >> dataPath;
		std::cin.clear();
	} while (!std::filesystem::exists(dataPath));

	std::string deviceId = "";
	std::string devicePath = "";
	for(;;)
	{
		std::cout << "Input device ID: ";
		std::cin >> deviceId;
		std::cin.clear();
		devicePath = dataPath + "/" + deviceId;
		if (!std::filesystem::exists(devicePath))
		{
			continue;
		}

		std::cout << "Files found..." << std::endl;
		Ingester ingest(devicePath);
		std::array<T_ErrorBarF, 2> EisData = ingest.ReadEISFiles();

		Grapher grapher(devicePath + "/Plots");
		grapher.GraphEIS(deviceId, EisData[0], EisData[1]);
	}
}

void GraphEIS(std::string path, std::string Id)
{
	std::cout << "Graphing EIS for " << Id << "..." << std::endl;

}

void GraphCV(std::string path, std::string Id)
{

}

void GraphCIL(std::string path, std::string Id)
{

}
