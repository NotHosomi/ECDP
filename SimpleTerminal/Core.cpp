#include "Core.h"
#include <iostream>
#include <algorithm>
#include "Ingester.h"
#include "TerminalColours.h"
#include "PrintTable.h"
#include "StrUtils.h"
#include "JsonLoader.h"
#include "Commands.h"
#include "Options.h"
#include "BatchData.h"
#include "PopulationVariance.h"


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
	std::string plotDir = m_tUserConfig.plotDirectory;
	if (plotDir == "")
	{
		plotDir = "./Plots";
	}
	m_Grapher.SetOutputPath(plotDir);
}

E_CmdErr Core::Run(const std::string sDeviceId, E_DataTypes eModes)
{
	std::filesystem::path devicePath = UserConfig().dataDirectory + "/" + sDeviceId;
	if (!std::filesystem::exists(devicePath))
	{
		std::cout << "Could not find " << sDeviceId << std::endl;
		return E_CmdErr::BadArgs;
	}


	std::cout << "\nReading device " << sDeviceId << "\n-------------------" << std::endl;
	Ingester ingest(devicePath);


	// todo: pull from archive
	T_DeviceData data = m_Archive.GetDevice(sDeviceId);
	bool isInArchive = data.sDeviceId != "";
	if (isInArchive)
	{
		std::cout << "Device " << sDeviceId << "found in archive" << std::endl;
	}
	else
	{
		data.sDeviceId = sDeviceId;
		data.tInfo = ingest.GetDeviceInfo();
	}

	// compare
	// See if the archived data is missing components
	// run the missing components
	// add the data to the archive


	if (eModes & E_DataTypes::kEis)
	{
		Eis(data, ingest, m_tUserConfig.eis);
	}
	if (eModes & E_DataTypes::kCv)
	{
		Cv(data, ingest, m_tUserConfig.cv);
	}
	if (eModes & E_DataTypes::kCil)
	{
		Cil(data, ingest, m_tUserConfig.cil);
	}

	if (!isInArchive || Options::Get().GetOpt<bool>("arch-overwrite"))
	{
		m_Archive.AddDevice(data);
	}
	m_Archive.SaveAll();

	std::cout << "\nFinished " << sDeviceId << "\n" << std::endl;
	return E_CmdErr::None;
}

T_EisData Core::Eis(T_DeviceData& tDeviceData, const Ingester& ingest, const T_EisConfig& tConfig)
{
	// EIS
	if (!tDeviceData.tEis.has_value() || Options::Get().GetOpt<bool>("arch-overwrite") || Options::Get().GetOpt<bool>("arch-recalc"))
	{
		std::cout << "\nFetching EIS values...";
		T_EisData newdata = ingest.ParseEis(tConfig.keyVals);
		if (newdata.mImpedances.size() == 0)
		{
			std::cout << "No EIS data found" << std::endl;
		}
		else
		{
			tDeviceData.tEis = newdata;
		}
	}

	if (tDeviceData.tEis.value().mImpedances.size() == 0)
	{
		return {};
	}
	// EIS Table
	PrintEisVals(tDeviceData.tEis.value(), tConfig);

	// EIS Plot
	if (std::get<int>(Options::Get().GetOpt("eis-plot-avrg").val) == 1)
	{
		std::array<T_ErrorBarD, 2> EisData = ingest.GetEisPlot();
		m_Grapher.GraphDeviceEIS(tDeviceData.sDeviceId, EisData[0], EisData[1]);
	}
	if (tConfig.plotEachElectrode)
	{
		// todo: add this to the grapher
	}
	return tDeviceData.tEis.value();
}

T_CvData Core::Cv(T_DeviceData& tDeviceData, const Ingester& ingest, const T_CvConfig& tConfig)
{
	if (!tConfig.calcCsc) { return {}; }


	if (!tDeviceData.tCv.has_value() || Options::Get().GetOpt<bool>("arch-overwrite") || Options::Get().GetOpt<bool>("arch-recalc"))
	{
		T_CvData newdata = ingest.CalculateCscVals();
		if (newdata.mElectrodes.size() == 0)
		{
			std::cout << "No CV data found" << std::endl;
		}
		else
		{
			tDeviceData.tCv = newdata;
		}
	}

	if (tDeviceData.tCv.value().mElectrodes.size() == 0)
	{
		return {};
	}	
	//std::vector<std::string> cvExcludes;
	PrintCscVals(tDeviceData.tCv.value());

	// Plot each CV
	if (tConfig.plotEachElectrode)
	{
		for (const auto& [key, data] : tDeviceData.tCv.value().mElectrodes)
		{
			m_Grapher.GraphElectrodeCV(tDeviceData.sDeviceId, key, data);
		}
	}
	
	// Plot aggregate CV
	if (tConfig.plotCv)
	{
		T_ErrorBarD tCvPlot = ingest.GetCvPlot();
		m_Grapher.GraphDeviceCV(tDeviceData.sDeviceId, tCvPlot);
	}
	return tDeviceData.tCv.value();
}

T_CilData Core::Cil(T_DeviceData& tDeviceData, const Ingester& ingest, const T_CilConfig& tConfig)
{
	if (!tConfig.calcCil)
	{
		return {};
	}

	if (!tDeviceData.tCil.has_value() || Options::Get().GetOpt<bool>("arch-overwrite") || Options::Get().GetOpt<bool>("arch-recalc"))
	{
		T_CilData newdata = ingest.CalculateCilVals();
		
		if (newdata.mCilVals.size() == 0)
		{
			std::cout << "No CIL data found" << std::endl;
		}
		else
		{
			tDeviceData.tCil = newdata;
		}
	}

	const T_CilData& tCilData = tDeviceData.tCil.value();
	if (tCilData.vPulseWidths.size() == 0)
	{
		return {};
	}
	std::cout << "\nCIL values" << std::endl;
	PrintCilVals(tCilData.vPulseWidths, tCilData.mCilVals, tCilData.vCilStats);
	std::cout << "\nNormalised CIL values" << std::endl;
	PrintCilVals(tCilData.vPulseWidths, tCilData.mCilValsNormalised, tCilData.vCilStatsNormalised);

	if (tConfig.plotCil)
	{
		// Plot CIL values
		m_Grapher.GraphDeviceCIL(tDeviceData.sDeviceId, tCilData);
	}
	if (tConfig.plotEachElectrode)
	{
		// todo
	}
	return tCilData;
}

T_UserConfig& Core::UserConfig()
{
	return m_tUserConfig;
}

void Core::PrintEisVals(const T_EisData& tEisData, const T_EisConfig& tConfig)
{
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
		CscTable.AddRow({iter.first, std::to_string(iter.second.dCsc), std::to_string(iter.second.dCscNorm)}, colour);
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

T_DeviceData Core::BatchAverages(const std::vector<std::string> sIds)
{
	std::vector<T_DeviceData> devices;
	for (const auto& sId : sIds)
	{
		if (m_Archive.GetDevice(sId).sDeviceId == "")
		{
			std::cout << "\nFound device " << sId << " in archive" << std::endl;
			devices.push_back(m_Archive.GetDevice(sId));
		}
		else
		{
			std::cout << "\nDevice " << sId << " not in archive" << std::flush;
			if(Run(sId, static_cast<E_DataTypes>(E_DataTypes::kEis | E_DataTypes::kCv | E_DataTypes::kCil)) == E_CmdErr::NoData)
			{
				devices.push_back(m_Archive.GetDevice(sId));
			}			
		}
	}

	// EIS
	std::map<std::string, std::vector<T_StatGroup>> eisStats;
	for (auto& device : devices)
	{
		if (!device.tEis.has_value())
		{
			continue;
		}
		const T_EisData& eis = device.tEis.value();
		for (int i = 0; i < eis.vFrequencies.size(); ++i)
		{
			T_StatGroup group;
			group.n = eis.mImpedances.size();
			group.mean = eis.vAverages[i];
			group.sd = eis.vStddev[i];
			eisStats[eis.vFrequencies[i]].push_back(group);
		}
	}
	std::map<std::string, T_Stats> eisBatchAvrg;
	for (const auto& [freq, groupList] : eisStats)
	{
		eisBatchAvrg.insert({ freq, PooledStddev(groupList) });
	}
	

	// CV
	std::vector<T_StatGroup> cvStats;
	std::vector<T_StatGroup> cvNormStats;
	for (const auto& device : devices)
	{
		if (device.tCv.has_value())
		{
			continue;
		}
		const T_CvData& cv = device.tCv.value();
		T_StatGroup group;
		group.n = cv.mElectrodes.size();
		group.mean = cv.tCsc.mean;
		group.sd = cv.tCsc.stddev;
		cvStats.push_back(group);
		group.mean = cv.tCscNorm.mean;
		group.sd = cv.tCscNorm.stddev;
		cvNormStats.push_back(group);
	}
	T_Stats cvBatchStats = PooledStddev(cvStats);
	T_Stats cvNormBatchStats = PooledStddev(cvNormStats);
	
	// CIL
	std::map<int, std::vector<T_StatGroup>> cilStats;
	std::map<int, std::vector<T_StatGroup>> cilNormStats;
	for (const auto& device : devices)
	{
		if (device.tCv.has_value())
		{
			continue;
		}
		const T_CilData& cil = device.tCil.value();
		T_StatGroup group;
		group.n = cil.mCilVals.size();
		for (int i = 0; i < cil.vPulseWidths.size(); ++i)
		{
			group.mean = cil.vCilStats[i].mean;
			group.sd = cil.vCilStats[i].stddev;
			cilStats[cil.vPulseWidths[i]].push_back(group);
			group.mean = cil.vCilStatsNormalised[i].mean;
			group.sd = cil.vCilStatsNormalised[i].stddev;
			cilNormStats[cil.vPulseWidths[i]].push_back(group);
		}
	}
}