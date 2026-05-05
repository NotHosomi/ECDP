#include "Core.h"
#include <iostream>
#include <algorithm>
#include "Ingester.h"
#include "TerminalColours.h"
#include "PrintTable.h"
#include "StrUtils.h"
#include "JsonLoader.h"
#include "Commands.h"


Core::Core()
{
	std::cout << "Loading user config..." << std::flush;
	if (!LoadJson<T_UserConfig>("./UserConfig.json", m_tUserConfig))
	{
		std::cout << "Failed -- Creating default..." << std::flush;
		SaveJson("./UserConfig.json", m_tUserConfig);
	}
	std::cout << "Done" << std::endl;

	// Validate data dir
	m_DataPath = m_tUserConfig.dataDirectory;
	if (!std::filesystem::exists(m_DataPath))
	{
		do
		{
			if (m_DataPath != "")
			{
				std::cout << TERM_BOLDRED << "Data path \"" + m_DataPath.string() + "\" does not exist" << TERM_RESET << std::endl;
			}
			std::cout << "Input data path: ";
			std::cin >> m_DataPath;
			std::cin.clear();
		} while (!std::filesystem::exists(m_DataPath));
	}
	else
	{
		std::cout << "Data directory: " << std::filesystem::path(m_DataPath) << std::endl;
	}
	m_Grapher.SetOutputPath(m_tUserConfig.plotDirectory);
}

bool Core::Run(const std::string sDeviceId, E_DataTypes eModes)
{
	std::filesystem::path devicePath = UserConfig().dataDirectory + "/" + sDeviceId;
	if (!std::filesystem::exists(devicePath))
	{
		std::cout << "Could not find " << sDeviceId << std::endl;
		return E_CmdErr::BadArgs;
	}


	std::cout << "\nReading device " << sDeviceId << "\n-------------------" << std::endl;
	Ingester ingest(devicePath);

	if (eModes & E_DataTypes::kEis)
	{
		Eis(sDeviceId, ingest, UserConfig().eis);
	}
	if (eModes & E_DataTypes::kCv)
	{
		Cv(sDeviceId, ingest, UserConfig().cv);
	}
	if (eModes & E_DataTypes::kCil)
	{
		Cil(sDeviceId, ingest, UserConfig().cil);
	}

	std::cout << "\nFinished " << sDeviceId << "\n" << std::endl;
	return true;
}

void Core::Eis(const std::string sDeviceId, const Ingester& ingest, const T_EisConfig& tConfig)
{
	// EIS
	std::cout << "\nFetching EIS values..." << std::endl;
	T_EisData tEisData = ingest.ParseEis(tConfig.keyVals);

	// EIS Table
	std::vector<std::string> headers;
	headers.push_back("Electrode");
	for (auto keyval : tConfig.keyVals)
	{
		headers.push_back(keyval + " Hz");
	}
	PrintTable EisTable(headers);
	std::vector<std::string> eisExclusionList;
	std::vector<std::string> newRow;
	std::string colour;
	for (const auto& iter : tEisData.mImpedances)
	{
		colour = TERM_GREEN;
		if (std::any_of(iter.second.begin(), iter.second.end(), [tConfig](double val) { return val > tConfig.maxValidImpedance; }))
		{
			colour = TERM_RED;
		}
		EisTable.AddRow(iter.first, iter.second, colour);
	}
	colour = TERM_GREEN;
	if (std::any_of(tEisData.vAverages.begin(), tEisData.vAverages.end(), [tConfig](double val) { return val > tConfig.maxValidImpedance; }))
	{
		colour = TERM_RED;
	}
	EisTable.AddRow("AVRG", tEisData.vAverages);
	EisTable.AddRow("STDDEV", tEisData.vStddev);
	EisTable.Print(TERM_BOLDRED);


	// EIS Plot
	if (tConfig.plotEis)
	{
		std::array<T_ErrorBarD, 2> EisData = ingest.GetEisPlot();
		m_Grapher.GraphDeviceEIS(sDeviceId, EisData[0], EisData[1]);
	}
	if (tConfig.plotEachElectrode)
	{
		// todo: add this to the grapher
	}
}

void Core::Cv(const std::string sDeviceId, const Ingester& ingest, const T_CvConfig& tConfig)
{
	if (!tConfig.calcCsc) { return; }

	std::vector<std::string> cvExcludes;
	T_CvData tCv = ingest.CalculateCscVals();
	PrintCscVals(tCv);

	// Plot each CV
	if (tConfig.plotEachElectrode)
	{
		for (const auto& [key, data] : tCv.mElectrodes)
		{
			m_Grapher.GraphElectrodeCV(sDeviceId, key, data);
		}
	}
	
	// Plot aggregate CV
	if (tConfig.plotCv)
	{
		T_ErrorBarD tCvPlot = ingest.GetCvPlot(cvExcludes);
		m_Grapher.GraphDeviceCV(sDeviceId, tCvPlot);
	}
}

void Core::Cil(const std::string sDeviceId, const Ingester& ingest, const T_CilConfig& tConfig)
{
	if (tConfig.calcCil)
	{
		T_CilData cils = ingest.CalculateCilVals();
		if (cils.vPulseWidths.size() == 0)
		{
			return;
		}
		std::cout << "\nCIL values" << std::endl;
		PrintCilVals(cils.vPulseWidths, cils.mCilVals, cils.vCilStats);
		std::cout << "\nNormalised CIL values" << std::endl;
		PrintCilVals(cils.vPulseWidths, cils.mCilValsNormalised, cils.vCilStatsNormalised);

		if (tConfig.plotCil)
		{
			// Plot CIL values
			m_Grapher.GraphDeviceCIL(sDeviceId, cils);
		}
		if (tConfig.plotEachElectrode)
		{

		}
	}
}

T_UserConfig& Core::UserConfig()
{
	return m_tUserConfig;
}

void Core::PrintCscVals(const T_CvData& tCvData)
{
	PrintTable CscTable({ "Electrode", "CSC (mC)", "Normalised (mC/cm^2)" });
	double sum = 0.0;
	for (const auto& iter : tCvData.mElectrodes)
	{
		std::string colour = "";
		if (iter.second.dCscNorm < 1 || iter.second.dCscNorm != iter.second.dCscNorm)
		{
			colour = TERM_RED;
		}
		CscTable.AddRow({ iter.first, std::to_string(iter.second.dCscNorm) }, colour);
		sum += iter.second.dCscNorm;
	}
	CscTable.AddRow("AVRG", std::vector<double>({ tCvData.tCsc.mean, tCvData.tCscNorm.mean }));
	CscTable.AddRow("STDDEV", std::vector<double>({ tCvData.tCsc.stddev, tCvData.tCscNorm.stddev }));
	CscTable.Print(TERM_BOLDBLUE);
}

void Core::PrintCilVals(std::vector<int> vPulseWidths, std::map<int, std::vector<float>> mVals, std::vector<T_Stats> vStats)
{
	std::vector<std::string> cilTableHeaders{ "Electrode #" };
	for (const auto& pulseWidth : vPulseWidths) { cilTableHeaders.push_back(std::to_string(pulseWidth) + "us"); };
	PrintTable cilTable(cilTableHeaders);
	for (const auto& row : mVals)
	{
		std::vector<std::string> rowtext{ std::to_string(row.first) };
		for (const auto& val : row.second) { rowtext.push_back(std::to_string(val)); };
		cilTable.AddRow(rowtext);
	}
	std::vector<std::string> avrgRowText{ "{AVRG}" };
	std::vector<std::string> stddevRowText{ "{STDDEV}" };
	for (const auto& stats : vStats)
	{
		avrgRowText.push_back(std::to_string(stats.mean));
		stddevRowText.push_back(std::to_string(stats.stddev));
	};
	cilTable.AddRow(avrgRowText);
	cilTable.AddRow(stddevRowText);
	cilTable.Print(TERM_YELLOW);
}
