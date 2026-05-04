#include "Core.h"
#include <iostream>
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

void Core::Eis(const std::string sDeviceId, const Ingester& ingest, const T_EisConfig& tConfig)
{
	if (!tConfig.fetchKeyvals) { return; }

	// EIS
	std::cout << "\nFetching EIS values..." << std::endl;
	std::map<std::string, std::vector<double>> ImpedanceKeyvals = ingest.GetEisKeyvals(tConfig.keyVals);

	// EIS Table
	std::vector<std::string> headers;
	headers.push_back("Electrode");
	for (auto keyval : tConfig.keyVals)
	{
		headers.push_back(keyval + " Hz");
	}
	PrintTable EisTable(headers);
	std::vector<std::string> eisExclusionList;
	int validCount = 0;
	for (const auto& iter : ImpedanceKeyvals)
	{
		std::string colour = TERM_RED;
		if (iter.second[1] <= tConfig.maxValidImpedance)
		{
			validCount += 1;
			colour = TERM_GREEN;
		}
		EisTable.AddRow({ iter.first, SU::RoundToStr(iter.second[0]), SU::RoundToStr(iter.second[1]), SU::RoundToStr(iter.second[2]) }, colour);
	}
	EisTable.Print(TERM_BOLDRED);


	// EIS Plot
	if (tConfig.plotEis)
	{
		std::array<T_ErrorBarD, 2> EisData = ingest.GetEisPlot();
		m_Grapher.GraphEIS(sDeviceId, EisData[0], EisData[1]);
	}
	if (tConfig.plotEis)
	{
		// todo: add this to the grapher
	}
}

void Core::Cv(const std::string sDeviceId, const Ingester& ingest, const T_CvConfig& tConfig)
{
	std::vector<std::string> cvExcludes;
	if (tConfig.calcCsc)
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

		// Plot each CV
		if (tConfig.plotEachElectrode)
		{
			for (const auto& [key, data] : mCv)
			{
				m_Grapher.GraphCV(sDeviceId, key, data);
			}
		}
	}

	// Plot aggregate CV
	if (tConfig.plotCv)
	{
		T_ErrorBarD tCvPlot = ingest.GetCvPlot(cvExcludes);
		m_Grapher.GraphCV(sDeviceId, tCvPlot);
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
			m_Grapher.GraphCIL(sDeviceId, cils);
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
